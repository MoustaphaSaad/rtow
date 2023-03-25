package main

import "core:math/linalg"
import "core:math"
import "core:math/rand"

MaterialRecord :: struct {
	attenuation: Color,
	scattered:   Ray,
}

Lambertian :: struct {
	albedo: Color,
}

lambertian_scatter :: proc(self: Lambertian, r: Ray, rec: HitRecord) -> (res: MaterialRecord, scattered: bool) {
	scatter_direction := rec.normal + random_unit_vector()
	if v3_near_zero(scatter_direction) {
		scatter_direction = rec.normal
	}
	res.scattered = Ray{rec.p, scatter_direction}
	res.attenuation = self.albedo
	scattered = true
	return
}

Metal :: struct {
	albedo: Color,
	fuzz: f32,
}

metal_scatter :: proc(self: Metal, r: Ray, rec: HitRecord) -> (res: MaterialRecord, scattered: bool) {
	reflected := v3_reflect(v3_normalize(r.Dir), rec.normal)
	res.scattered = Ray{rec.p, reflected + v3_splat(self.fuzz) * random_in_unit_sphere()}
	res.attenuation = self.albedo
	scattered = v3_dot(res.scattered.Dir, rec.normal) > 0
	return
}

Dielectric :: struct {
	ir: f32,
}

dielectric_scatter :: proc(self: Dielectric, r: Ray, rec: HitRecord) -> (res: MaterialRecord, scattered: bool) {
	res.attenuation = Color{1, 1, 1}
	refraction_ratio := self.ir
	if rec.front_face {
		refraction_ratio = 1 / self.ir
	}

	unit_direction := v3_normalize(r.Dir)
	cos_theta := math.min(v3_dot(-unit_direction, rec.normal), 1.0)
	sin_theta := math.sqrt(1 - cos_theta * cos_theta)

	cannot_refract := refraction_ratio * sin_theta > 1
	direction: Vec3
	if cannot_refract || relfectance(cos_theta, refraction_ratio) > rand.float32() {
		direction = v3_reflect(unit_direction, rec.normal)
	} else {
		direction = v3_refract(unit_direction, rec.normal, refraction_ratio)
	}
	res.scattered = Ray{rec.p, direction}
	scattered = true
	return
}

relfectance :: proc(cosine, ref_idx: f32) -> f32 {
	r0 := (1 - ref_idx) / (1 + ref_idx)
	r0 = r0*r0
	return r0 + (1 - r0) * math.pow((1 - cosine), 5)
}

Material :: union #no_nil {
	Lambertian,
	Metal,
	Dielectric,
}

material_scatter :: proc(self: Material, r: Ray, rec: HitRecord) -> (MaterialRecord, bool) {
	switch m in self {
	case Lambertian: return lambertian_scatter(m, r, rec)
	case Metal: return metal_scatter(m, r, rec)
	case Dielectric: return dielectric_scatter(m, r, rec)
	case: return MaterialRecord{}, false
	}
}