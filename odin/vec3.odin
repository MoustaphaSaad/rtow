package main

import "core:io"
import "core:fmt"
import "core:math"
import "core:builtin"

Vec3 :: [3]f64
Color :: Vec3
Point3 :: Vec3

write_color :: proc(out: io.Writer, c: Color, samples_per_pixel: f64) {
	r := c.r
	g := c.g
	b := c.b

	scale := 1.0 / samples_per_pixel
	r *= scale
	g *= scale
	b *= scale

	fmt.wprintf(out, "%v %v %v\n", int(256 * math.clamp(r, 0, 0.999)), int(256 * math.clamp(g, 0, 0.999)), int(256 * math.clamp(b, 0, 0.999)))
}
