package main

type HitRecord struct {
	P         Point3
	Normal    Vec3
	T         float64
	FrontFace bool
	Mat       Material
}

func (h *HitRecord) setFaceNormal(r Ray, outwardNormal Vec3) {
	h.FrontFace = r.Dir.Dot(outwardNormal) < 0
	if h.FrontFace {
		h.Normal = outwardNormal
	} else {
		h.Normal = outwardNormal.Negate()
	}
}

type Hittable interface {
	Hit(r Ray, tMin, tMax float64) (HitRecord, bool)
}