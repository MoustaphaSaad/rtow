package main

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

func (m *Material) Scatter(series *RandomSeries, rIn Ray, rec *HitRecord) (attenuation Color, scattered *Ray) {
	switch m.Kind {
	case MaterialKindLambertian:
		return m.ScatterLambertian(series, rIn, rec)
	case MaterialKindMetal:
		return m.ScatterMetal(series, rIn, rec)
	case MaterialKindDielectric:
		return m.ScatterDielectric(series, rIn, rec)
	default:
		return
	}
}

func (m *Material) ScatterLambertian(series *RandomSeries, rIn Ray, rec *HitRecord) (attenuation Color, scattered *Ray) {
	scatterDirection := rec.Normal.Add(RandomUnitVector(series))

	if scatterDirection.NearZero() {
		scatterDirection = rec.Normal
	}

	scattered = &Ray{
		Orig: rec.P,
		Dir:  scatterDirection,
	}
	attenuation = m.Albedo
	return
}

func (m *Material) ScatterMetal(series *RandomSeries, rIn Ray, rec *HitRecord) (attenuation Color, scattered *Ray) {
	reflected := rIn.Dir.UnitVector().Reflect(rec.Normal)
	dir := reflected.Add(RandomInUnitSphere(series).Mul(m.Fuzz))
	if dir.Dot(rec.Normal) > 0 {
		scattered = &Ray{
			Orig: rec.P,
			Dir:  dir,
		}
	}
	attenuation = m.Albedo
	return
}

func (m *Material) ScatterDielectric(series *RandomSeries, rIn Ray, rec *HitRecord) (attenuation Color, scattered *Ray) {
	attenuation = Color{1, 1, 1}
	refraction_ratio := m.IndexOfRefraction
	if rec.FrontFace {
		refraction_ratio = 1 / m.IndexOfRefraction
	}

	unit_direction := rIn.Dir.UnitVector()
	cos_theta := Min(unit_direction.Negate().Dot(rec.Normal), 1)
	sin_theta := Sqrt(1 - cos_theta*cos_theta)

	cannot_refract := refraction_ratio*sin_theta > 1
	var direction Vec3
	if cannot_refract || Reflectance(cos_theta, refraction_ratio) > RandomDouble(series) {
		direction = unit_direction.Reflect(rec.Normal)
	} else {
		direction = unit_direction.Refract(rec.Normal, refraction_ratio)
	}

	scattered = &Ray{rec.P, direction}
	return
}

func Reflectance(cosine, ref_idx Scalar) Scalar {
	r0 := (1 - ref_idx) / (1 + ref_idx)
	r0 = r0 * r0
	return r0 + (1-r0)*Pow((1-cosine), 5)
}
