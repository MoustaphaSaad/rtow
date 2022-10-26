package main

import "core:math"
import "core:math/linalg"

Sphere :: struct {
	center: Point3,
	radius: f64,
}

sphere_hit :: proc(self: ^Sphere, r: Ray, t_min, t_max: f64) -> (rec: HitRecord, hit: bool) {
	a := linalg.length2(r.Dir)
	oc := r.Orig - self.center
	half_b := linalg.dot(oc, r.Dir)
	c := linalg.length2(oc) - self.radius * self.radius
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
	return
}

sphere_to_hittable :: proc(self: ^Sphere) -> Hittable {
	return Hittable {
		data = self,
		vtable = &_sphere_vtable,
	}
}

_sphere_vtable := Hittable_VTable {
	hit = proc(self: Hittable, r: Ray, t_min, t_max: f64) -> (HitRecord, bool) {
		sphere := (^Sphere)(self.data)
		return sphere_hit(sphere, r, t_min, t_max)
	},
}