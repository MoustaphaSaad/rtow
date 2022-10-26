package main

import "core:math/linalg"

HitRecord :: struct {
	p: Point3,
	normal: Vec3,
	t: f64,
	front_face: bool,
}

hit_record_set_face_normal :: proc(self: ^HitRecord, r: Ray, outward_normal: Vec3) {
	self.front_face = linalg.dot(r.Dir, outward_normal) < 0
	if self.front_face {
		self.normal = outward_normal
	} else {
		self.normal = -outward_normal
	}
}

Hittable :: struct {
	using vtable: ^Hittable_VTable,
	data: rawptr,
}
Hittable_VTable :: struct {
	hit: proc(self: Hittable, r: Ray, t_min, t_max: f64) -> (HitRecord, bool),
}
