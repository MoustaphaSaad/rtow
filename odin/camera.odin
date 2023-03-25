package main

import "core:math"
import "core:math/linalg"

Camera :: struct {
	origin, lower_left_corner: Point3,
	horizontal, vertical: Vec3,
	u, v, w: Vec3,
	lens_radius: f32,
}

new_camera :: proc(lookfrom, lookat: Point3, vup: Vec3, vfov, aspect_ratio, aperture, focus_dist: f32) -> (cam: Camera) {
	theta := degrees_to_radians(vfov)
	h := math.tan(theta / 2)
	viewport_height := 2 * h
	viewport_width := aspect_ratio * viewport_height

	cam.w = linalg.normalize(lookfrom - lookat)
	cam.u = linalg.normalize(linalg.cross(vup, cam.w))
	cam.v = linalg.cross(cam.w, cam.u)

	cam.origin = lookfrom
	cam.horizontal = cam.u * viewport_width * focus_dist
	cam.vertical = cam.v * viewport_height * focus_dist
	cam.lower_left_corner = cam.origin - cam.horizontal/2 - cam.vertical/2 - focus_dist * cam.w

	cam.lens_radius = aperture / 2;
	return
}

camera_ray :: proc(self: Camera, s, t: f32) -> Ray {
	rd := self.lens_radius * random_in_unit_disk()
	offset := self.u * rd.x + self.v * rd.y
	return Ray {
		Orig = self.origin + offset,
		Dir = self.lower_left_corner + self.horizontal * s + self.vertical * t - self.origin - offset,
	}
}