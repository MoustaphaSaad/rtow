package main

type HitRecord struct {
	P      Point3
	Normal Vec3
	T      float64
}

type Hittable interface {
	Hit(r Ray, tMin, tMax float64) (HitRecord, bool)
}