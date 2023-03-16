package main

import "core:math/linalg"

MaterialRecord :: struct {
	attenuation: Color,
	scattered:   Ray,
}

Material :: struct {
	using vtable: ^Material_VTable,
	data: rawptr,
}

Material_VTable :: struct {
	scatter: proc(self: Material, r: Ray, rec: HitRecord) -> (MaterialRecord, bool),
}

Lambertian :: struct {
	albedo: Color,
}

lambertian_scatter :: proc(self: ^Lambertian, r: Ray, rec: HitRecord) -> (res: MaterialRecord, scattered: bool) {
	scatter_direction := rec.normal + random_unit_vector()
	if vec3_near_zero(scatter_direction) {
		scatter_direction = rec.normal
	}
	res.scattered = Ray{rec.p, scatter_direction}
	res.attenuation = self.albedo
	scattered = true
	return
}

lambertian_to_material :: proc(self: ^Lambertian) -> Material {
	return Material {
		data = self,
		vtable = &_lambertian_vtable,
	}
}

_lambertian_vtable := Material_VTable {
	scatter = proc(self: Material, r: Ray, rec: HitRecord) -> (MaterialRecord, bool) {
		lambertian := (^Lambertian)(self.data)
		return lambertian_scatter(lambertian, r, rec)
	},
}

Metal :: struct {
	albedo: Color,
	fuzz: f64,
}

metal_scatter :: proc(self: ^Metal, r: Ray, rec: HitRecord) -> (res: MaterialRecord, scattered: bool) {
	reflected := linalg.reflect(linalg.normalize(r.Dir), rec.normal)
	res.scattered = Ray{rec.p, reflected + self.fuzz * random_in_unit_sphere()}
	res.attenuation = self.albedo
	scattered = linalg.dot(res.scattered.Dir, rec.normal) > 0
	return
}

metal_to_material :: proc(self: ^Metal) -> Material {
	return Material {
		data = self,
		vtable = &_metal_vtable,
	}
}

_metal_vtable := Material_VTable {
	scatter = proc(self: Material, r: Ray, rec: HitRecord) -> (MaterialRecord, bool) {
		metal := (^Metal)(self.data)
		return metal_scatter(metal, r, rec)
	},
}

Dielectric :: struct {
	ir: f64,
}

dielectric_scatter :: proc(self: ^Dielectric, r: Ray, rec: HitRecord) -> (res: MaterialRecord, scattered: bool) {
	res.attenuation = Color{1, 1, 1}
	refraction_ratio := self.ir
	if rec.front_face {
		refraction_ratio = 1 / self.ir
	}

	unit_direction := linalg.normalize(r.Dir)
	refracted := linalg.refract(unit_direction, rec.normal, refraction_ratio)
	res.scattered = Ray{rec.p, refracted}
	scattered = true
	return
}

dielectric_to_material :: proc(self: ^Dielectric) -> Material {
	return Material {
		data = self,
		vtable = &_dielectric_vtable,
	}
}

_dielectric_vtable := Material_VTable {
	scatter = proc(self: Material, r: Ray, rec: HitRecord) -> (MaterialRecord, bool) {
		dielectric := (^Dielectric)(self.data)
		return dielectric_scatter(dielectric, r, rec)
	},
}

to_material :: proc{lambertian_to_material, metal_to_material, dielectric_to_material}
