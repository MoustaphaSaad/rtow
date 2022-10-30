package main

type Camera struct {
	Origin, LowerLeftCorner Point3
	Horizontal, Vertical    Vec3
}

func NewCamera() (cam Camera) {
	aspectRatio := 16.0 / 9.0
	viewportHeight := 2.0
	viewportWidth := aspectRatio * viewportHeight
	focalLength := 1.0

	cam.Origin = Point3{0, 0, 0}
	cam.Horizontal = Vec3{viewportWidth, 0, 0}
	cam.Vertical = Vec3{0, viewportHeight, 0}
	cam.LowerLeftCorner = cam.Origin.Sub(cam.Horizontal.Div(2)).Sub(cam.Vertical.Div(2)).Sub(Vec3{0, 0, focalLength})
	return
}

func (cam Camera) ray(u, v float64) Ray {
	return Ray{
		Orig: cam.Origin,
		Dir:  cam.LowerLeftCorner.Add(cam.Horizontal.Mul(u)).Add(cam.Vertical.Mul(v)).Sub(cam.Origin),
	}
}