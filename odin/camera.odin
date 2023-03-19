package main

import "core:math"
import "core:math/linalg"

Camera :: struct {
	origin, lower_left_corner: Point3,
	horizontal, vertical: Vec3,
}

new_camera :: proc(lookfrom, lookat: Point3, vup: Vec3, vfov, aspect_ratio: f64) -> (cam: Camera) {
	theta := degrees_to_radians(vfov)
	h := math.tan(theta / 2)
	viewport_height := 2 * h
	viewport_width := aspect_ratio * viewport_height

	w := linalg.normalize(lookfrom - lookat)
	u := linalg.normalize(linalg.cross(vup, w))
	v := linalg.cross(w, u)

	cam.origin = lookfrom
	cam.horizontal = u * viewport_width
	cam.vertical = v * viewport_height
	cam.lower_left_corner = cam.origin - cam.horizontal/2 - cam.vertical/2 - w
	return
}

camera_ray :: proc(self: Camera, s, t: f64) -> Ray {
	return Ray {
		Orig = self.origin,
		Dir = self.lower_left_corner + self.horizontal * s + self.vertical * t - self.origin,
	}
}