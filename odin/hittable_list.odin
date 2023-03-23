package main

import "core:fmt"

HittableList :: struct {
	allocations: [dynamic]rawptr,
	hittables: [dynamic]Hittable,
}

hittable_list_new :: proc() -> (res: ^HittableList) {
	res = new(HittableList)
	res.allocations = make([dynamic]rawptr)
	res.hittables = make([dynamic]Hittable)
	return
}

hittable_list_free :: proc(self: ^HittableList) {
	for a in self.allocations {
		free(a)
	}
	delete(self.allocations)
	delete(self.hittables)
	free(self)
}

hittable_list_alloc :: proc(self: ^HittableList, d: $T) -> (res: ^T) {
	res = new(T)
	res^ = d
	append(&self.allocations, res)
	return res
}

hittable_list_add :: proc(self: ^HittableList, h: $T) {
	append(&self.hittables, to_hittable(hittable_list_alloc(self, h)))
}

hittable_list_hit :: proc(self: ^HittableList, r: Ray, t_min, t_max: f64) -> (rec: HitRecord, hit: bool) {
	hit = false
	closest_so_far := t_max

	for v in self.hittables {
		if v_rec, v_hit := v->hit(r, t_min, closest_so_far); v_hit {
			hit = true
			closest_so_far = v_rec.t
			rec = v_rec
		}
	}

	return
}

hittable_list_to_hittable :: proc(self: ^HittableList) -> Hittable {
	return Hittable {
		data = self,
		vtable = &_hittable_list_vtable,
	}
}

_hittable_list_vtable := Hittable_VTable {
	hit = proc(self: Hittable, r: Ray, t_min, t_max: f64) -> (HitRecord, bool) {
		list := (^HittableList)(self.data)
		return hittable_list_hit(list, r, t_min, t_max)
	},
}

to_hittable :: proc{hittable_list_to_hittable, sphere_to_hittable}