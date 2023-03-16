package main

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
	refracted := unit_direction.Refract(rec.Normal, refraction_ratio)

	res.Scattered = Ray{rec.P, refracted}
	scattered = true
	return
}