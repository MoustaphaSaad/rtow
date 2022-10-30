package main

import (
	"fmt"
	"io"
	"math"
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
	res[0] = u[1] * v[2] - u[2] * v[1]
	res[1] = u[2] * v[0] - u[0] * v[2]
	res[2] = u[0] * v[1] - u[1] * v[0]
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
	r *= scale
	g *= scale
	b *= scale
	fmt.Fprintf(
		out,
		"%v %v %v\n",
		int(256 * Clamp(r, 0.0, 0.999)),
		int(256 * Clamp(g, 0.0, 0.999)),
		int(256 * Clamp(b, 0.0, 0.999)),
	)
}