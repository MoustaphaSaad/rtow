package main

import "core:fmt"

HittableList :: struct {
	allocations: [dynamic]rawptr,
	spheres: [dynamic]Sphere,
}

hittable_list_new :: proc() -> (res: ^HittableList) {
	res = new(HittableList)
	res.allocations = make([dynamic]rawptr)
	res.spheres = make([dynamic]Sphere)
	return
}

hittable_list_free :: proc(self: ^HittableList) {
	for a in self.allocations {
		free(a)
	}
	delete(self.allocations)
	delete(self.spheres)
	free(self)
}

hittable_list_alloc :: proc(self: ^HittableList, d: $T) -> (res: ^T) {
	res = new(T)
	res^ = d
	append(&self.allocations, res)
	return res
}

hittable_list_add :: proc(self: ^HittableList, s: Sphere) {
	append(&self.spheres, s)
}

hittable_list_hit :: proc(self: ^HittableList, r: Ray, t_min, t_max: f64) -> (rec: HitRecord, hit: bool) {
	hit = false
	closest_so_far := t_max

	for v in self.spheres {
		if v_rec, v_hit := sphere_hit(v, r, t_min, closest_so_far); v_hit {
			hit = true
			closest_so_far = v_rec.t
			rec = v_rec
		}
	}

	return
}
