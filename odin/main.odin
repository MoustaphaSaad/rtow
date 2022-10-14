package main

import "core:fmt"
import "core:os"
import "core:time"
import "core:bufio"
import "core:io"

main :: proc() {
	start := time.now()

	buffered_writer: bufio.Writer
	bufio.writer_init(&buffered_writer, io.to_writer(os.stream_from_handle(os.stdout)))
	defer {
		bufio.writer_flush(&buffered_writer)
		bufio.writer_destroy(&buffered_writer)
	}
	writer := io.to_writer(bufio.writer_to_stream(&buffered_writer))

	image_width :: 256
	image_height :: 256

	fmt.wprintf(writer, "P3\n%v %v\n255\n", image_width, image_height)

	for j := image_height - 1; j >= 0; j -= 1 {
		fmt.fprintf(os.stderr, "\rElapsed time: %v ms, ", time.duration_milliseconds(time.since(start)))
		fmt.fprintf(os.stderr, "Scanlines remaining: %v", j)
		for i in 0..<image_width {
			r := f64(i) / (image_width - 1)
			g := f64(j) / (image_height - 1)
			b := 0.25

			ir := int(255.999 * r)
			ig := int(255.999 * g)
			ib := int(255.999 * b)

			fmt.wprintf(writer, "%v %v %v\n", ir, ig, ib)
		}
	}

	fmt.fprintf(os.stderr, "\nDone.\n")
	fmt.fprintf(os.stderr, "Elapsed time: %v ms\n", time.duration_milliseconds(time.since(start)))
}