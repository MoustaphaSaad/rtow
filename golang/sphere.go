package main

type Sphere struct {
	Center        Point3
	Radius        Scalar
	MaterialIndex int
}

func (s Sphere) Hit(r Ray, tMin, tMax Scalar) (rec HitRecord, hit bool) {
	a := r.Dir.LengthSquared()
	oc := r.Orig.Sub(s.Center)
	halfB := oc.Dot(r.Dir)
	c := oc.LengthSquared() - s.Radius*s.Radius
	discriminant := halfB*halfB - a*c
	if discriminant < 0 {
		hit = false
		return
	}
	sqrtd := Sqrt(discriminant)

	// find the nearest of 2 possible solutions
	root := (-halfB - sqrtd) / a
	if root < tMin || root > tMax {
		root = (-halfB + sqrtd) / a
		if root < tMin || root > tMax {
			hit = false
			return
		}
	}

	hit = true
	rec.T = root
	rec.P = r.At(rec.T)
	outwardNormal := rec.P.Sub(s.Center).Div(s.Radius)
	rec.setFaceNormal(r, outwardNormal)
	rec.MaterialIndex = s.MaterialIndex
	return
}