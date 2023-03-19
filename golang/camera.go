package main

import "math"

type Camera struct {
	Origin, LowerLeftCorner Point3
	Horizontal, Vertical    Vec3
}

func NewCamera(lookfrom, lookat Point3, vup Vec3, vfov, aspectRatio float64) (cam Camera) {
	theta := degressToRadians(vfov)
	h := math.Tan(theta / 2)
	viewportHeight := 2 * h
	viewportWidth := aspectRatio * viewportHeight

	w := lookfrom.Sub(lookat).UnitVector()
	u := vup.Cross(w).UnitVector()
	v := w.Cross(u)

	cam.Origin = lookfrom
	cam.Horizontal = u.Mul(viewportWidth)
	cam.Vertical = v.Mul(viewportHeight)
	cam.LowerLeftCorner = cam.Origin.Sub(cam.Horizontal.Div(2)).Sub(cam.Vertical.Div(2)).Sub(w)
	return
}

func (cam Camera) ray(s, t float64) Ray {
	return Ray{
		Orig: cam.Origin,
		Dir:  cam.LowerLeftCorner.Add(cam.Horizontal.Mul(s)).Add(cam.Vertical.Mul(t)).Sub(cam.Origin),
	}
}