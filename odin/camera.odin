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

	cam.w = v3_normalize(lookfrom - lookat)
	cam.u = v3_normalize(v3_cross(vup, cam.w))
	cam.v = v3_cross(cam.w, cam.u)

	cam.origin = lookfrom
	cam.horizontal = cam.u * v3_splat(viewport_width * focus_dist)
	cam.vertical = cam.v * v3_splat(viewport_height * focus_dist)
	cam.lower_left_corner = cam.origin - cam.horizontal * v3_splat(0.5) - cam.vertical * v3_splat(0.5) - v3_splat(focus_dist) * cam.w

	cam.lens_radius = aperture / 2;
	return
}

camera_ray :: proc(self: Camera, s, t: f32) -> Ray {
	rd := v3_splat(self.lens_radius) * random_in_unit_disk()
	offset := self.u * v3_splat_x(rd) + self.v * v3_splat_y(rd)
	return Ray {
		Orig = self.origin + offset,
		Dir = self.lower_left_corner + self.horizontal * v3_splat(s) + self.vertical * v3_splat(t) - self.origin - offset,
	}
}