package main

Camera :: struct {
	origin, lower_left_corner: Point3,
	horizontal, vertical: Vec3,
}

new_camera :: proc() -> (cam: Camera) {
	aspect_ratio := 16.0 / 9.0
	viewport_height := 2.0
	viewport_width := aspect_ratio * viewport_height
	focal_length := 1.0

	cam.origin = Point3{0, 0, 0}
	cam.horizontal = Vec3{viewport_width, 0, 0}
	cam.vertical = Vec3{0, viewport_height, 0}
	cam.lower_left_corner = cam.origin - cam.horizontal/2 - cam.vertical/2 - Vec3{0, 0, focal_length}
	return
}

camera_ray :: proc(self: Camera, u, v: f64) -> Ray {
	return Ray {
		Orig = self.origin,
		Dir = self.lower_left_corner + self.horizontal * u + self.vertical * v - self.origin,
	}
}