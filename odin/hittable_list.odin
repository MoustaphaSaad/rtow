package main

import "core:fmt"

HittableList :: struct {
	materials: [dynamic]Material,
	spheres: [dynamic]Sphere,
}

hittable_list_new :: proc() -> (res: ^HittableList) {
	res = new(HittableList)
	res.materials = make([dynamic]Material)
	res.spheres = make([dynamic]Sphere)
	return
}

hittable_list_free :: proc(self: ^HittableList) {
	delete(self.materials)
	delete(self.spheres)
	free(self)
}

hittable_list_add_material :: proc(self: ^HittableList, m: Material) -> int {
	append(&self.materials, m)
	return len(self.materials) - 1
}

hittable_list_add_sphere :: proc(self: ^HittableList, s: Sphere) {
	append(&self.spheres, s)
}

hittable_list_hit :: proc(self: ^HittableList, r: Ray, t_min, t_max: f32) -> (rec: HitRecord, hit: bool) {
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
