package main

//go:generate ispc -g --target=sse2,sse4,avx1,avx2 spheres_hit.ispc -o ./build/spheres_hit.o -h ./build/spheres_hit.h

type HittableList struct {
	materials []Material
	spheres   []Sphere
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
