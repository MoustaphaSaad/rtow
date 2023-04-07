package main

import (
	"fmt"
	"io"
	"math"
)

type Scalar = float32

var infinity = Scalar(math.Inf(1))

func Tan(v Scalar) Scalar {
	return Scalar(math.Tan(float64(v)))
}

func Sqrt(v Scalar) Scalar {
	return Scalar(math.Sqrt(float64(v)))
}

func Abs(v Scalar) Scalar {
	if v < 0 {
		return v * -1
	} else {
		return v
	}
}

func Min(v1, v2 Scalar) Scalar {
	if v1 < v2 {
		return v1
	} else {
		return v2
	}
}

func Max(v1, v2 Scalar) Scalar {
	if v1 > v2 {
		return v1
	} else {
		return v2
	}
}

func Pow(b, e Scalar) Scalar {
	return Scalar(math.Pow(float64(b), float64(e)))
}

func Sin(a Scalar) Scalar {
	return Scalar(math.Sin(float64(a)))
}

func Cos(a Scalar) Scalar {
	return Scalar(math.Cos(float64(a)))
}



type RandomSeries struct {
	state uint32
}

func (series *RandomSeries) Rand() uint32 {
	x := series.state
	x ^= x << 13
	x ^= x >> 17
	x ^= x << 15
	series.state = x
	return x
}

func Rand(series *RandomSeries) Scalar {
	return Scalar(series.Rand()) / Scalar(^uint32(0))
}



type Vec3 struct {
	X, Y, Z Scalar
}

func (v Vec3) Negate() (res Vec3) {
	res.X = -v.X
	res.Y = -v.Y
	res.Z = -v.Z
	return
}

func (v Vec3) Add(u Vec3) (res Vec3) {
	res.X = v.X + u.X
	res.Y = v.Y + u.Y
	res.Z = v.Z + u.Z
	return
}

func (v Vec3) Sub(u Vec3) (res Vec3) {
	res.X = v.X - u.X
	res.Y = v.Y - u.Y
	res.Z = v.Z - u.Z
	return
}

func (v Vec3) Mul(t Scalar) (res Vec3) {
	res.X = v.X * t
	res.Y = v.Y * t
	res.Z = v.Z * t
	return
}

func (v Vec3) HMul(u Vec3) (res Vec3) {
	res.X = v.X * u.X
	res.Y = v.Y * u.Y
	res.Z = v.Z * u.Z
	return
}

func (v Vec3) Dot(u Vec3) Scalar {
	return u.X * v.X + u.Y * v.Y + u.Z * v.Z
}

func (v Vec3) Cross(u Vec3) (res Vec3) {
	res.X = v.Y * u.Z - v.Z * u.Y
	res.Y = v.Z * u.X - v.X * u.Z
	res.Z = v.X * u.Y - v.Y * u.X
	return
}

func (v Vec3) Div(t Scalar) (res Vec3) {
	return v.Mul(1 / t)
}

func (v Vec3) Length() Scalar {
	return Sqrt(v.LengthSquared())
}

func (v Vec3) LengthSquared() Scalar {
	return v.X * v.X + v.Y * v.Y + v.Z * v.Z
}

func (v Vec3) UnitVector() Vec3 {
	return v.Div(v.Length())
}

func (v Vec3) NearZero() bool {
	const s = 1e-8
	return (Abs(v.X) < s) && (Abs(v.Y) < s) && (Abs(v.Z) < s)
}

func (v Vec3) Reflect(normal Vec3) Vec3 {
	return v.Sub(normal.Mul(v.Dot(normal) * 2))
}

func (uv Vec3) Refract(normal Vec3, etai_over_etat Scalar) Vec3 {
	cos_theta := Min(uv.Negate().Dot(normal), 1)
	r_out_perp := uv.Add(normal.Mul(cos_theta)).Mul(etai_over_etat)
	r_out_parallel := normal.Mul(-Sqrt(Abs(1 - r_out_perp.LengthSquared())))
	return r_out_perp.Add(r_out_parallel)
}


func RandomDouble(series *RandomSeries) Scalar {
	return Rand(series)
}

func RandomDoubleInRange(series *RandomSeries, min, max Scalar) Scalar {
	return min + RandomDouble(series) * (max - min)
}

func RandomVec3(series *RandomSeries) Vec3 {
	return Vec3{Rand(series), Rand(series), Rand(series)}
}

func RandomVec3InRange(series *RandomSeries, min, max Scalar) Vec3 {
	return Vec3{RandomDoubleInRange(series, min, max), RandomDoubleInRange(series, min, max), RandomDoubleInRange(series, min, max)}
}

func RandomInUnitSphere(series *RandomSeries) Vec3 {
	z := RandomDoubleInRange(series, -1, 1)
	t := RandomDoubleInRange(series, 0, 2 * pi)
	r := Sqrt(Max(0.0, 1.0 - z * z))
	x := r * Cos(t)
	y := r * Sin(t)
	res := Vec3{x, y, z}
	res = res.Mul(Pow(RandomDouble(series), 1.0 / 3.0))
	return res;
}

func RandomUnitVector(series *RandomSeries) Vec3 {
	z := RandomDoubleInRange(series, -1, 1)
	a := RandomDoubleInRange(series, 0, 2 * pi)
	r := Sqrt(Scalar(1) - z * z)
	x := r * Cos(a)
	y := r * Sin(a)
	return Vec3 {x, y, z}
}

func RandomInHemisphere(series *RandomSeries, normal Vec3) Vec3 {
	inUnitSphere := RandomInUnitSphere(series)
	if inUnitSphere.Dot(normal) > 0.0 {
		return inUnitSphere
	} else {
		return inUnitSphere.Negate()
	}
}

type Point3 = Vec3
type Color = Vec3

func Clamp(x, min, max Scalar) Scalar {
	if x < min { return min }
	if x > max { return max }
	return x
}

func (c Color) Write(out io.Writer, samplesPerPixel Scalar) {
	r := c.X
	g := c.Y
	b := c.Z

	scale := 1.0 / samplesPerPixel
	r = Sqrt(scale * r)
	g = Sqrt(scale * g)
	b = Sqrt(scale * b)
	fmt.Fprintf(
		out,
		"%v %v %v\n",
		int(256 * Clamp(r, 0.0, 0.999)),
		int(256 * Clamp(g, 0.0, 0.999)),
		int(256 * Clamp(b, 0.0, 0.999)),
	)
}

func RandomInUnitDisk(series *RandomSeries) Vec3 {
	for {
		p := Vec3{RandomDoubleInRange(series, -1, 1), RandomDoubleInRange(series, -1, 1), 0}
		if p.LengthSquared() >= 1 { continue }
		return p
	}
}