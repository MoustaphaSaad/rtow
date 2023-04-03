package main

import "core:io"
import "core:fmt"
import "core:math"

Image :: struct {
	pixels: [dynamic]Color,
	width, height: int
}

image_new :: proc(w, h: int) -> (res: Image) {
	res.pixels = make([dynamic]Color, w * h)
	res.width  = w
	res.height = h
	return
}

image_free :: proc(self: Image) {
	delete(self.pixels)
}

image_index :: proc(self: Image, x, y: int) -> int {
	return y * self.width + x
}

image_write :: proc(self: Image, out: io.Writer) {
	fmt.wprintf(out, "P3\n%v %v\n255\n", self.width, self.height)
	for j := self.height - 1; j >= 0; j -= 1 {
		for i in 0 ..< self.width {
			ix := image_index(self, i, j)
			c_arr := v3_to_array(self.pixels[ix])
			r := c_arr[0]
			g := c_arr[1]
			b := c_arr[2]

			fmt.wprintf(out, "%v %v %v\n", int(256 * math.clamp(r, 0, 0.999)), int(256 * math.clamp(g, 0, 0.999)), int(256 * math.clamp(b, 0, 0.999)))
		}
	}
}