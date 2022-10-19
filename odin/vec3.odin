package main

import "core:io"
import "core:fmt"

Vec3 :: [3]f64
Color :: Vec3
Point3 :: Vec3

write_color :: proc(out: io.Writer, c: Color) {
	fmt.wprintf(
		out,
		"%v %v %v\n",
		int(255.999 * c[0]),
		int(255.999 * c[1]),
		int(255.999 * c[2]),
	)
}
