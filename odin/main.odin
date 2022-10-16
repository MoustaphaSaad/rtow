package main

import "core:fmt"
import "core:os"
import "core:time"
import "core:bufio"
import "core:io"

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

	image_width :: 256
	image_height :: 256

	fmt.wprintf(stdout, "P3\n%v %v\n255\n", image_width, image_height)

	for j := image_height - 1; j >= 0; j -= 1 {
		fmt.wprintf(stderr, "\rElapsed time: %v ms, ", time.duration_milliseconds(time.since(start)))
		fmt.wprintf(stderr, "Scanlines remaining: %v", j)
		for i in 0..<image_width {
			r := f64(i) / (image_width - 1)
			g := f64(j) / (image_height - 1)
			b := 0.25

			ir := int(255.999 * r)
			ig := int(255.999 * g)
			ib := int(255.999 * b)

			fmt.wprintf(stdout, "%v %v %v\n", ir, ig, ib)
		}
	}

	fmt.wprintf(stderr, "\nDone.\n")
	fmt.wprintf(stderr, "Elapsed time: %v ms\n", time.duration_milliseconds(time.since(start)))
}