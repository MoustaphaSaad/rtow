package main

import "math"

type Camera struct {
	Origin, LowerLeftCorner Point3
	Horizontal, Vertical    Vec3
	U, V, W Vec3
	LensRadius float64
}

func NewCamera(lookfrom, lookat Point3, vup Vec3, vfov, aspectRatio, aperture, focusDist float64) (cam Camera) {
	theta := degressToRadians(vfov)
	h := math.Tan(theta / 2)
	viewportHeight := 2 * h
	viewportWidth := aspectRatio * viewportHeight

	cam.W = lookfrom.Sub(lookat).UnitVector()
	cam.U = vup.Cross(cam.W).UnitVector()
	cam.V = cam.W.Cross(cam.U)

	cam.Origin = lookfrom
	cam.Horizontal = cam.U.Mul(viewportWidth).Mul(focusDist)
	cam.Vertical = cam.V.Mul(viewportHeight).Mul(focusDist)
	cam.LowerLeftCorner = cam.Origin.Sub(cam.Horizontal.Div(2)).Sub(cam.Vertical.Div(2)).Sub(cam.W.Mul(focusDist))

	cam.LensRadius = aperture / 2;
	return
}

func (cam Camera) ray(s, t float64) Ray {
	rd := RandomInUnitDisk().Mul(cam.LensRadius)
	offset := cam.U.Mul(rd.X()).Add(cam.V.Mul(rd.Y()))

	return Ray{
		Orig: cam.Origin.Add(offset),
		Dir:  cam.LowerLeftCorner.Add(cam.Horizontal.Mul(s)).Add(cam.Vertical.Mul(t)).Sub(cam.Origin).Sub(offset),
	}
}