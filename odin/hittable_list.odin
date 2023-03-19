package main

import "core:fmt"

HittableList :: []Hittable

hittable_list_hit :: proc(self: HittableList, r: Ray, t_min, t_max: f64) -> (rec: HitRecord, hit: bool) {
	hit = false
	closest_so_far := t_max

	for v in self {
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
		return hittable_list_hit(list^, r, t_min, t_max)
	},
}