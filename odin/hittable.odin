package main

import "core:math/linalg"

HitRecord :: struct {
	p: Point3,
	normal: Vec3,
	t: f64,
	front_face: bool,
	material_index: int,
}

hit_record_set_face_normal :: proc(self: ^HitRecord, r: Ray, outward_normal: Vec3) {
	self.front_face = linalg.dot(r.Dir, outward_normal) < 0
	if self.front_face {
		self.normal = outward_normal
	} else {
		self.normal = -outward_normal
	}
}
