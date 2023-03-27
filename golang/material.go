package main

type MaterialResult struct {
	Attenuation Color
	Scattered   Ray
}

type MaterialKind int

const (
	MaterialKindLambertian MaterialKind = iota
	MaterialKindMetal
	MaterialKindDielectric
)

type Material struct {
	Kind              MaterialKind
	Albedo            Color
	Fuzz              Scalar
	IndexOfRefraction Scalar
}

func Lambertian(albedo Color) (res Material) {
	res.Kind = MaterialKindLambertian
	res.Albedo = albedo
	return
}

func Metal(albedo Color, fuzz Scalar) (res Material) {
	res.Kind = MaterialKindMetal
	res.Albedo = albedo
	res.Fuzz = fuzz
	return
}

func Dielectric(indexOfRefraction Scalar) (res Material) {
	res.Kind = MaterialKindDielectric
	res.IndexOfRefraction = indexOfRefraction
	return
}

func (m *Material) Scatter(rIn Ray, rec *HitRecord) (res MaterialResult, scattered bool) {
	switch m.Kind {
	case MaterialKindLambertian:
		return m.ScatterLambertian(rIn, rec)
	case MaterialKindMetal:
		return m.ScatterMetal(rIn, rec)
	case MaterialKindDielectric:
		return m.ScatterDielectric(rIn, rec)
	default:
		scattered = false
		return
	}
}

func (m *Material) ScatterLambertian(rIn Ray, rec *HitRecord) (res MaterialResult, scattered bool) {
	scatterDirection := rec.Normal.Add(RandomUnitVector())

	if scatterDirection.NearZero() {
		scatterDirection = rec.Normal
	}

	res.Scattered = Ray{
		Orig: rec.P,
		Dir:  scatterDirection,
	}
	res.Attenuation = m.Albedo
	scattered = true
	return
}

func (m *Material) ScatterMetal(rIn Ray, rec *HitRecord) (res MaterialResult, scattered bool) {
	reflected := rIn.Dir.UnitVector().Reflect(rec.Normal)
	res.Scattered = Ray{
		Orig: rec.P,
		Dir:  reflected.Add(RandomInUnitSphere().Mul(m.Fuzz)),
	}
	res.Attenuation = m.Albedo
	scattered = res.Scattered.Dir.Dot(rec.Normal) > 0
	return
}

func (m *Material) ScatterDielectric(rIn Ray, rec *HitRecord) (res MaterialResult, scattered bool) {
	res.Attenuation = Color{1, 1, 1}
	refraction_ratio := m.IndexOfRefraction
	if rec.FrontFace {
		refraction_ratio = 1 / m.IndexOfRefraction
	}

	unit_direction := rIn.Dir.UnitVector()
	cos_theta := Min(unit_direction.Negate().Dot(rec.Normal), 1)
	sin_theta := Sqrt(1 - cos_theta*cos_theta)

	cannot_refract := refraction_ratio*sin_theta > 1
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

func Reflectance(cosine, ref_idx Scalar) Scalar {
	r0 := (1 - ref_idx) / (1 + ref_idx)
	r0 = r0 * r0
	return r0 + (1-r0)*Pow((1-cosine), 5)
}
