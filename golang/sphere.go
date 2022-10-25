package main

import "math"

type Sphere struct {
	Center Point3
	Radius float64
}

func (s Sphere) Hit(r Ray, tMin, tMax float64) (rec HitRecord, hit bool) {
	a := r.Dir.LengthSquared()
	oc := r.Orig.Sub(s.Center)
	halfB := oc.Dot(r.Dir)
	c := oc.LengthSquared() - s.Radius*s.Radius
	discriminant := halfB*halfB - a*c
	if discriminant < 0 {
		hit = false
		return
	}
	sqrtd := math.Sqrt(discriminant)

	// find the nearest of 2 possible solutions
	root := (-halfB - sqrtd) / a;
	if root < tMin || root > tMax {
		root = (-halfB + sqrtd) / a;
		if root < tMin || root > tMax {
			hit = false
			return
		}
	}

	hit = true
	rec.T = root
	rec.P = r.At(rec.T)
	rec.Normal = rec.P.Sub(s.Center).Div(s.Radius)
	return
}