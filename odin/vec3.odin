package main

import "core:math"
import "core:io"
import "core:fmt"

Vec3 :: [3]f64

length :: proc(v: Vec3) -> f64 {
	return math.sqrt(length_squared(v))
}

length_squared :: proc(v: Vec3) -> f64 {
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]
}

dot :: proc(u, v: Vec3) -> f64 {
	return u[0] * v[0] + u[1] * v[1] + u[2] * v[2]
}

cross :: proc(u, v: Vec3) -> Vec3 {
	return {
		u[1] * v[2] - u[2] * v[1],
		u[2] * v[0] - u[0] * v[2],
		u[0] * v[1] - u[1] * v[0],
	};
}

unit_vector :: proc(v: Vec3) -> Vec3 {
	return v / length(v)
}

Color :: distinct Vec3
Point3 :: distinct Vec3

write_color :: proc(out: io.Writer, c: Color) {
	fmt.wprintf(
		out,
		"%v %v %v\n",
		int(255.999 * c[0]),
		int(255.999 * c[1]),
		int(255.999 * c[2]),
	)
}
