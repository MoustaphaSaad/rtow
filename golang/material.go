package main

import "math"

type MaterialResult struct {
	Attenuation Color
	Scattered   Ray
}

type Material interface {
	Scatter(rIn Ray, rec HitRecord) (MaterialResult, bool)
}

type Lambertian struct {
	Albedo Color
}

func (l Lambertian) Scatter(rIn Ray, rec HitRecord) (res MaterialResult, scattered bool) {
	scatterDirection := rec.Normal.Add(RandomUnitVector())

	if scatterDirection.NearZero() {
		scatterDirection = rec.Normal
	}

	res.Scattered = Ray{
		Orig: rec.P,
		Dir:  scatterDirection,
	}
	res.Attenuation = l.Albedo
	scattered = true
	return
}

type Metal struct {
	Albedo Color
	Fuzz   float64
}

func (m Metal) Scatter(rIn Ray, rec HitRecord) (res MaterialResult, scattered bool) {
	reflected := rIn.Dir.UnitVector().Reflect(rec.Normal)
	res.Scattered = Ray{
		Orig: rec.P,
		Dir:  reflected.Add(RandomInUnitSphere().Mul(m.Fuzz)),
	}
	res.Attenuation = m.Albedo
	scattered = res.Scattered.Dir.Dot(rec.Normal) > 0
	return
}

type Dielectric struct {
	IndexOfRefraction float64
}

func (d Dielectric) Scatter(rIn Ray, rec HitRecord) (res MaterialResult, scattered bool) {
	res.Attenuation = Color{1, 1, 1}
	refraction_ratio := d.IndexOfRefraction
	if rec.FrontFace {
		refraction_ratio = 1 / d.IndexOfRefraction
	}

	unit_direction := rIn.Dir.UnitVector()
	cos_theta := math.Min(unit_direction.Negate().Dot(rec.Normal), 1)
	sin_theta := math.Sqrt(1 - cos_theta*cos_theta)

	cannot_refract := refraction_ratio * sin_theta > 1
	var direction Vec3
	if cannot_refract || Reflectance(cos_theta, refraction_ratio) > RandomDouble() {
		direction = unit_direction.Reflect(rec.Normal)
	} else {
		direction = unit_direction.Refract(rec.Normal, refraction_ratio)
	}

	res.Scattered = Ray{rec.P, direction}
	scattered = true
	return
}

func Reflectance(cosine, ref_idx float64) float64 {
	r0 := (1 - ref_idx) / (1 + ref_idx)
	r0 = r0 * r0
	return r0 + (1 - r0) * math.Pow((1 - cosine), 5)
}
