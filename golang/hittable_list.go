package main

//go:generate ispc --pic -g --target=sse2,sse4,avx1,avx2 spheres_hit.ispc -o ./build/spheres_hit.o
//go:generate ar -rcs ./build/spheres_hit.a ./build/*.o

// #cgo LDFLAGS: ./build/spheres_hit.a
// #include "spheres_hit.h"
import "C"
import "unsafe"

type SpheresSOA struct {
	centerX, centerY, centerZ, radius []Scalar
	matIndex []int
}

type HittableList struct {
	materials []Material
	spheres   []Sphere
	soa SpheresSOA
}

func (list *HittableList) AddSphere(h Sphere) {
	list.spheres = append(list.spheres, h)
}

func (list *HittableList) AddMaterial(m Material) int {
	list.materials = append(list.materials, m)
	return len(list.materials) - 1
}

func (list *HittableList) Clear() {
	list.spheres = make([]Sphere, 0)
}

func roundUp(num, factor int) int {
	if factor == 0 { return 0 }
	if num % factor == 0 { return num }
	return num + factor - 1 - (num + factor - 1) % factor
}

func (list *HittableList) PrepareSOA() {
	simdCount := roundUp(len(list.spheres), 8)
	list.soa.centerX = make([]Scalar, simdCount)
	list.soa.centerY = make([]Scalar, simdCount)
	list.soa.centerZ = make([]Scalar, simdCount)
	list.soa.radius = make([]Scalar, simdCount)
	list.soa.matIndex = make([]int, simdCount)
	for i := 0; i < len(list.spheres); i++ {
		list.soa.centerX[i] = list.spheres[i].Center.X
		list.soa.centerY[i] = list.spheres[i].Center.Y
		list.soa.centerZ[i] = list.spheres[i].Center.Z
		list.soa.radius[i] = list.spheres[i].Radius
		list.soa.matIndex[i] = list.spheres[i].MaterialIndex
	}
}

func (list *HittableList) HitSOA(r Ray, tMin, tMax Scalar) (rec *HitRecord) {
	ispcRay := C.Ray{}
	ispcRay.origin.v[0] = C.float(r.Orig.X)
	ispcRay.origin.v[1] = C.float(r.Orig.Y)
	ispcRay.origin.v[2] = C.float(r.Orig.Z)
	ispcRay.dir.v[0] = C.float(r.Dir.X)
	ispcRay.dir.v[1] = C.float(r.Dir.Y)
	ispcRay.dir.v[2] = C.float(r.Dir.Z)

	var outT C.float
	var outHitIndex C.int

	res := C.spheres_hit(
		(*C.float)(unsafe.Pointer(&list.soa.centerX[0])),
		(*C.float)(unsafe.Pointer(&list.soa.centerY[0])),
		(*C.float)(unsafe.Pointer(&list.soa.centerZ[0])),
		(*C.float)(unsafe.Pointer(&list.soa.radius[0])),
		C.int(len(list.spheres)),
		(*C.Ray)(unsafe.Pointer(&ispcRay)),
		C.float(tMin),
		C.float(tMax),
		(*C.float)(unsafe.Pointer(&outT)),
		(*C.int)(unsafe.Pointer(&outHitIndex)),
	)

	if res == false {
		return
	}

	rec = &HitRecord{}
	rec.T = float32(outT)
	rec.P = r.At(rec.T)
	center := Vec3{list.soa.centerX[outHitIndex], list.soa.centerY[outHitIndex], list.soa.centerZ[outHitIndex]}
	outwardNormal := rec.P.Sub(center).Div(list.soa.radius[outHitIndex])
	rec.setFaceNormal(r, outwardNormal)
	rec.MaterialIndex = list.soa.matIndex[outHitIndex]
	return rec
}

func (list *HittableList) Hit(r Ray, tMin, tMax Scalar) (rec *HitRecord) {
	closestSoFar := tMax

	for i := range list.spheres {
		if v_rec := list.spheres[i].Hit(r, tMin, closestSoFar); v_rec != nil {
			closestSoFar = v_rec.T
			rec = v_rec
		}
	}

	return rec
}
