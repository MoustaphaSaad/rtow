package main

import "core:math"
import "core:math/linalg"

Sphere :: struct {
	center: Point3,
	radius: f32,
	material_index: int,
}

sphere_hit :: proc(self: Sphere, r: Ray, t_min, t_max: f32) -> (rec: HitRecord, hit: bool) {
	a := v3_length2(r.Dir)
	oc := r.Orig - self.center
	half_b := v3_dot(oc, r.Dir)
	c := v3_length2(oc) - self.radius * self.radius
	discriminant := half_b * half_b - a * c
	if discriminant < 0 {
		hit = false
		return
	}
	sqrtd := math.sqrt(discriminant)

	// find the nearest of 2 possible solutions
	root := (-half_b - sqrtd) / a;
	if root < t_min || root > t_max {
		root = (-half_b + sqrtd) / a;
		if root < t_min || root > t_max {
			hit = false
			return
		}
	}

	hit = true
	rec.t = root
	rec.p = ray_at(r, rec.t)
	outward_normal := (rec.p - self.center) / self.radius
	hit_record_set_face_normal(&rec, r, outward_normal)
	rec.material_index = self.material_index
	return
}