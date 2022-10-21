package main

import "core:fmt"
import "core:os"
import "core:time"
import "core:bufio"
import "core:io"
import "core:math/linalg"
import "core:math"

hit_sphere :: proc(center: Point3, radius: f64, r: Ray) -> f64 {
	// sphere around arbitrary center equation is
	// P is a point in 3D space
	// (P - center)^2 = radius^2 -> (P.x - center.x)^2 + (P.y - center.y)^2 + (P.z - center.z)^2 = radius^2
	// if you replace P with (ray.origin + t * ray.direction) and simplify
	// you get a quadratic equation
	// t^2 * (ray.direction.x^2 + ray.direction.y^2 + ray.direction.z^2) +
	// 2t * (ray.direction.x * (ray.origin.x - center.x) + ray.direction.y * (ray.origin.y - center.y) + ray.direction.z * (ray.origin.z - center.z)) +
	// (ray.origin.x - center.x)^2 + (ray.origin.y - center.y)^2 + (ray.origin.z - center.z)^2 - radius^2 = 0
	// which means that
	// a = dot(ray.direction, ray.direction)
	// b = dot((ray.origin - center), ray.direction)
	// c = dot((ray.origin - center), (ray.origin - center)) - radius^2
	// and using the quadratic equation formula you have the discriminant = b^2 - 4ac, if it's positive we have 2 solutions
	// if it's 0 we have one, if it's negative we have no solution

	a := linalg.length2(r.Dir)
	oc := r.Orig - center
	half_b := linalg.dot(oc, r.Dir)
	c := linalg.length2(oc) - radius * radius
	discriminant := half_b * half_b - a * c
	if discriminant < 0 {
		return -1
	} else {
		return (-half_b - math.sqrt(discriminant)) / a
	}
}

ray_color :: proc(r: Ray) -> Color {
	t := hit_sphere(Point3{0, 0, -1}, 0.5, r)
	if t > 0 {
		N := linalg.normalize(ray_at(r, t) - Vec3{0, 0, -1})
		return 0.5 * (N + Vec3{1, 1, 1})
	}
	unit_direction := linalg.normalize(r.Dir)
	t = 0.5 * (unit_direction.y + 1)
	return Color{1, 1, 1} * (1 - t) + Color{0.5, 0.7, 1} * t
}

main :: proc() {
	start := time.now()

	buffered_stdout: bufio.Writer
	bufio.writer_init(&buffered_stdout, io.to_writer(os.stream_from_handle(os.stdout)))
	defer {
		bufio.writer_flush(&buffered_stdout)
		bufio.writer_destroy(&buffered_stdout)
	}
	stdout := io.to_writer(bufio.writer_to_stream(&buffered_stdout))

	buffered_stderr: bufio.Writer
	bufio.writer_init(&buffered_stderr, io.to_writer(os.stream_from_handle(os.stderr)))
	defer {
		bufio.writer_flush(&buffered_stderr)
		bufio.writer_destroy(&buffered_stderr)
	}
	stderr := io.to_writer(bufio.writer_to_stream(&buffered_stderr))

	// Image
	aspect_ratio := 16.0 / 9.0
	image_width := 400
	image_height := int(f64(image_width) / aspect_ratio)

	// Camera
	viewport_height := 2.0
	viewport_width := aspect_ratio * viewport_height
	focal_length := 1.0

	origin := Point3{0, 0, 0}
	horizontal := Vec3{viewport_width, 0, 0}
	vertical := Vec3{0, viewport_height, 0}
	lower_left_corner := origin - horizontal / 2 - vertical / 2 - Vec3{0, 0, focal_length}

	fmt.wprintf(stdout, "P3\n%v %v\n255\n", image_width, image_height)

	pixel_only: time.Duration

	for j := image_height - 1; j >= 0; j -= 1 {
		fmt.wprintf(
			stderr,
			"\rElapsed time: %v ms, ",
			time.duration_milliseconds(time.since(start)),
		)
		fmt.wprintf(stderr, "Scanlines remaining: %v", j)
		for i in 0 ..< image_width {
			start := time.now()
			u := f64(i) / f64(image_width - 1)
			v := f64(j) / f64(image_height - 1)

			r := Ray{origin, (lower_left_corner + u * horizontal + v * vertical) - origin}
			pixel_color := ray_color(r)
			pixel_only += time.since(start)

			write_color(stdout, pixel_color)
		}
	}

	fmt.wprintf(stderr, "\nDone.\n")
	fmt.wprintf(stderr, "Elapsed time: %v ms\n", time.duration_milliseconds(time.since(start)))
	fmt.wprintf(stderr, "Pixel time: %v ms\n", time.duration_milliseconds(pixel_only))
}
