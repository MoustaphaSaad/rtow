package main

import (
	"fmt"
	"math"
)

type Vec3 struct {
	E [3]float64
}

func (v Vec3) X() float64 {
	return v.E[0]
}

func (v Vec3) Y() float64 {
	return v.E[1]
}

func (v Vec3) Z() float64 {
	return v.E[2]
}

func (v Vec3) Negate() (res Vec3) {
	res.E[0] = -v.E[0]
	res.E[1] = -v.E[1]
	res.E[2] = -v.E[2]
	return
}

func (v Vec3) Add(u Vec3) (res Vec3) {
	res.E[0] = v.E[0] + u.E[0]
	res.E[1] = v.E[1] + u.E[1]
	res.E[2] = v.E[2] + u.E[2]
	return
}

func (v Vec3) Sub(u Vec3) (res Vec3) {
	res.E[0] = v.E[0] - u.E[0]
	res.E[1] = v.E[1] - u.E[1]
	res.E[2] = v.E[2] - u.E[2]
	return
}

func (v Vec3) Mul(t float64) (res Vec3) {
	res.E[0] = v.E[0] * t
	res.E[1] = v.E[1] * t
	res.E[2] = v.E[2] * t
	return
}

func (v Vec3) HMul(u Vec3) (res Vec3) {
	res.E[0] = v.E[0] * u.E[0]
	res.E[1] = v.E[1] * u.E[1]
	res.E[2] = v.E[2] * u.E[2]
	return
}

func (v Vec3) Dot(u Vec3) float64 {
	return u.E[0] * v.E[0] + u.E[1] * v.E[1] + u.E[2] * v.E[2]
}

func (v Vec3) Cross(u Vec3) (res Vec3) {
	res.E[0] = u.E[1] * v.E[2] - u.E[2] * v.E[1]
	res.E[1] = u.E[2] * v.E[0] - u.E[0] * v.E[2]
	res.E[2] = u.E[0] * v.E[1] - u.E[1] * v.E[0]
	return
}

func (v Vec3) Div(t float64) (res Vec3) {
	return v.Mul(1 / t)
}

func (v Vec3) Length() float64 {
	return math.Sqrt(v.LengthSquared())
}

func (v Vec3) LengthSquared() float64 {
	return v.E[0] * v.E[0] + v.E[1] * v.E[1] + v.E[2] * v.E[2]
}

func (v Vec3) UnitVector() Vec3 {
	return v.Div(v.Length())
}

type Point3 Vec3
type Color Vec3

func (c Color) String() string {
	return fmt.Sprintf(
		"%v %v %v",
		int(float64(255.999) * c.E[0]),
		int(float64(255.999) * c.E[1]),
		int(float64(255.999) * c.E[2]),
	)
}