package main

type HitRecord struct {
	P             Point3
	Normal        Vec3
	T             Scalar
	FrontFace     bool
	MaterialIndex int
}

func (h *HitRecord) setFaceNormal(r Ray, outwardNormal Vec3) {
	h.FrontFace = r.Dir.Dot(outwardNormal) < 0
	if h.FrontFace {
		h.Normal = outwardNormal
	} else {
		h.Normal = outwardNormal.Negate()
	}
}
