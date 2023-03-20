package main

import (
	"fmt"
	"io"
	"math"
	"math/rand"
)

type Vec3 [3]float64

func (v Vec3) X() float64 {
	return v[0]
}

func (v Vec3) Y() float64 {
	return v[1]
}

func (v Vec3) Z() float64 {
	return v[2]
}

func (v Vec3) Negate() (res Vec3) {
	res[0] = -v[0]
	res[1] = -v[1]
	res[2] = -v[2]
	return
}

func (v Vec3) Add(u Vec3) (res Vec3) {
	res[0] = v[0] + u[0]
	res[1] = v[1] + u[1]
	res[2] = v[2] + u[2]
	return
}

func (v Vec3) Sub(u Vec3) (res Vec3) {
	res[0] = v[0] - u[0]
	res[1] = v[1] - u[1]
	res[2] = v[2] - u[2]
	return
}

func (v Vec3) Mul(t float64) (res Vec3) {
	res[0] = v[0] * t
	res[1] = v[1] * t
	res[2] = v[2] * t
	return
}

func (v Vec3) HMul(u Vec3) (res Vec3) {
	res[0] = v[0] * u[0]
	res[1] = v[1] * u[1]
	res[2] = v[2] * u[2]
	return
}

func (v Vec3) Dot(u Vec3) float64 {
	return u[0] * v[0] + u[1] * v[1] + u[2] * v[2]
}

func (v Vec3) Cross(u Vec3) (res Vec3) {
	res[0] = v[1] * u[2] - v[2] * u[1]
	res[1] = v[2] * u[0] - v[0] * u[2]
	res[2] = v[0] * u[1] - v[1] * u[0]
	return
}

func (v Vec3) Div(t float64) (res Vec3) {
	return v.Mul(1 / t)
}

func (v Vec3) Length() float64 {
	return math.Sqrt(v.LengthSquared())
}

func (v Vec3) LengthSquared() float64 {
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]
}

func (v Vec3) UnitVector() Vec3 {
	return v.Div(v.Length())
}

func (v Vec3) NearZero() bool {
	const s = 1e-8
	return (math.Abs(v[0]) < s) && (math.Abs(v[1]) < s) && (math.Abs(v[2]) < s)
}

func (v Vec3) Reflect(normal Vec3) Vec3 {
	return v.Sub(normal.Mul(v.Dot(normal) * 2))
}

func (uv Vec3) Refract(normal Vec3, etai_over_etat float64) Vec3 {
	cos_theta := math.Min(uv.Negate().Dot(normal), 1)
	r_out_perp := uv.Add(normal.Mul(cos_theta)).Mul(etai_over_etat)
	r_out_parallel := normal.Mul(-math.Sqrt(math.Abs(1 - r_out_perp.LengthSquared())))
	return r_out_perp.Add(r_out_parallel)
}

func RandomDouble() float64 {
	return rand.Float64()
}

func RandomDoubleInRange(min, max float64) float64 {
	return min + RandomDouble() * (max - min)
}

func RandomVec3() Vec3 {
	return Vec3{rand.Float64(), rand.Float64(), rand.Float64()}
}

func RandomVec3InRange(min, max float64) Vec3 {
	return Vec3{RandomDoubleInRange(min, max), RandomDoubleInRange(min, max), RandomDoubleInRange(min, max)}
}

func RandomInUnitSphere() Vec3 {
	for {
		p := RandomVec3InRange(-1, 1)
		if p.LengthSquared() >= 1 { continue }
		return p
	}
}

func RandomUnitVector() Vec3 {
	return RandomInUnitSphere().UnitVector()
}

func RandomInHemisphere(normal Vec3) Vec3 {
	inUnitSphere := RandomInUnitSphere()
	if inUnitSphere.Dot(normal) > 0.0 {
		return inUnitSphere
	} else {
		return inUnitSphere.Negate()
	}
}

type Point3 = Vec3
type Color = Vec3

func Clamp(x, min, max float64) float64 {
	if x < min { return min }
	if x > max { return max }
	return x
}

func (c Color) Write(out io.Writer, samplesPerPixel float64) {
	r := c.X()
	g := c.Y()
	b := c.Z()

	scale := 1.0 / samplesPerPixel
	r = math.Sqrt(scale * r)
	g = math.Sqrt(scale * g)
	b = math.Sqrt(scale * b)
	fmt.Fprintf(
		out,
		"%v %v %v\n",
		int(256 * Clamp(r, 0.0, 0.999)),
		int(256 * Clamp(g, 0.0, 0.999)),
		int(256 * Clamp(b, 0.0, 0.999)),
	)
}

func RandomInUnitDisk() Vec3 {
	for {
		p := Vec3{RandomDoubleInRange(-1, 1), RandomDoubleInRange(-1, 1), 0}
		if p.LengthSquared() >= 1 { continue }
		return p
	}
}