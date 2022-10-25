package main

HitRecord :: struct {
	p: Point3,
	normal: Vec3,
	t: f64,
}

Hittable :: struct {
	using vtable: ^Hittable_VTable,
	data: rawptr,
}
Hittable_VTable :: struct {
	hit: proc(self: Hittable, r: Ray, t_min, t_max: f64) -> (HitRecord, bool),
}
