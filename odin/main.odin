package main

import "core:fmt"
import "core:os"
import "core:time"
import "core:bufio"
import "core:io"
import "core:math/linalg"

ray_color :: proc(r: Ray) -> Color {
	unit_direction := linalg.normalize(r.Dir)
	t := 0.5 * (unit_direction.y + 1)
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

	for j := image_height - 1; j >= 0; j -= 1 {
		fmt.wprintf(
			stderr,
			"\rElapsed time: %v ms, ",
			time.duration_milliseconds(time.since(start)),
		)
		fmt.wprintf(stderr, "Scanlines remaining: %v", j)
		for i in 0 ..< image_width {
			u := f64(i) / f64(image_width - 1)
			v := f64(j) / f64(image_height - 1)

			r := Ray{origin, (lower_left_corner + u * horizontal + v * vertical) - origin}
			pixel_color := ray_color(r)
			write_color(stdout, pixel_color)
		}
	}

	fmt.wprintf(stderr, "\nDone.\n")
	fmt.wprintf(stderr, "Elapsed time: %v ms\n", time.duration_milliseconds(time.since(start)))
}
