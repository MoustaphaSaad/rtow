package main

import "core:io"
import "core:fmt"
import "core:math"
import "core:builtin"
import "core:math/rand"
import "core:math/linalg"

Vec3 :: [3]f64
Color :: Vec3
Point3 :: Vec3

random_vec3 :: proc() -> Vec3 {
	return Vec3{rand.float64(), rand.float64(), rand.float64()}
}

random_vec3_in_range :: proc(min, max: f64) -> Vec3 {
	return Vec3{rand.float64_range(min, max), rand.float64_range(min, max), rand.float64_range(min, max)}
}

random_in_unit_sphere :: proc() -> Vec3 {
	for {
		p := random_vec3_in_range(-1, 1)
		if linalg.length2(p) >= 1 do continue
		return p
	}
}

random_unit_vector :: proc() -> Vec3 {
	return linalg.normalize(random_in_unit_sphere())
}

random_in_hemisphere :: proc(normal: Vec3) -> Vec3 {
	in_unit_sphere := random_in_unit_sphere()
	if linalg.dot(in_unit_sphere, normal) > 0 {
		return in_unit_sphere
	} else {
		return -in_unit_sphere
	}
}

write_color :: proc(out: io.Writer, c: Color, samples_per_pixel: f64) {
	r := c.r
	g := c.g
	b := c.b

	scale := 1.0 / samples_per_pixel
	r = math.sqrt(scale * r)
	g = math.sqrt(scale * g)
	b = math.sqrt(scale * b)

	fmt.wprintf(out, "%v %v %v\n", int(256 * math.clamp(r, 0, 0.999)), int(256 * math.clamp(g, 0, 0.999)), int(256 * math.clamp(b, 0, 0.999)))
}
