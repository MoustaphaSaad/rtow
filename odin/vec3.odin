package main

import "core:io"
import "core:fmt"
import "core:math"
import "core:builtin"
import "core:math/rand"
import "core:math/linalg"
import "core:simd"

SIMD_ENABLED :: #config(ENABLE_SIMD, false)

when !SIMD_ENABLED {
	Vec3 :: [3]f32
	Color :: Vec3
	Point3 :: Vec3

	v3_splat :: proc (a: f32) -> Vec3 {
		return Vec3 {a, a, a}
	}

	v3_dot :: linalg.dot
	v3_length2 :: linalg.length2
	v3_length :: linalg.length
	v3_normalize :: linalg.normalize
	v3_reflect :: linalg.reflect
	v3_refract :: linalg.refract
	v3_cross :: linalg.cross

	v3_splat_x :: proc (a: Vec3) -> Vec3 {
		return Vec3 {a.x, a.x, a.x}
	}

	v3_splat_y :: proc (a: Vec3) -> Vec3 {
		return Vec3 {a.y, a.y, a.y}
	}

	v3_sqrt :: proc (a: Vec3) -> Vec3 {
		return Vec3 {math.sqrt(a.x), math.sqrt(a.y), math.sqrt(a.z)}
	}

	v3_to_array :: proc (a: Vec3) -> (res: [4]f32) {
		res[0] = a.x
		res[1] = a.y
		res[2] = a.z
		return
	}

	v3_near_zero :: proc (v: Vec3) -> bool {
		s :: 1e-8
		return (math.abs(v[0]) < s) && (math.abs(v[1]) < s) && (math.abs(v[2]) < s)
	}
} else {
	Vec3 :: simd.f32x4
	Color :: Vec3
	Point3 :: Vec3

	v3_splat :: proc (a: f32) -> Vec3 {
		return Vec3 {a, a, a, a}
	}

	v3_dot :: proc (a, b: Vec3) -> f32 {
		return simd.reduce_add_ordered(a * b * Vec3{1, 1, 1, 0});
	}

	v3_length2 :: proc (a: Vec3) -> f32 {
		return v3_dot(a, a)
	}

	v3_length :: proc (a: Vec3) -> f32 {
		return math.sqrt(v3_length2(a))
	}

	v3_normalize :: proc (a: Vec3) -> Vec3 {
		return a * v3_splat(1.0 / v3_length(a))
	}

	v3_reflect :: proc (I, N: Vec3) -> Vec3 {
		b := N * v3_splat(2 * v3_dot(N, I))
		return I - b
	}

	v3_refract :: proc (I, Normal: Vec3, eta: f32) -> Vec3 {
		dv := v3_dot(Normal, I)
		k := 1 - eta*eta * (1 - dv*dv)
		a := I * v3_splat(eta)
		b := Normal * v3_splat(eta * dv + math.sqrt(k))
		return (a - b) * v3_splat(f32(int(k >= 0)))
	}

	v3_cross :: proc (a, b: Vec3) -> Vec3 {
		return simd.swizzle(simd.swizzle(a, 2, 0, 1, 3) * b - a * simd.swizzle(b, 2, 0, 1, 3), 2, 0, 1, 3)
	}

	v3_splat_x :: proc (a: Vec3) -> Vec3 {
		return simd.swizzle(a, 0, 0, 0, 0)
	}

	v3_splat_y :: proc (a: Vec3) -> Vec3 {
		return simd.swizzle(a, 1, 1, 1, 1)
	}

	v3_sqrt :: proc (a: Vec3) -> Vec3 {
		return simd.sqrt(a)
	}

	v3_to_array :: proc (a: Vec3) -> [4]f32 {
		return simd.to_array(a)
	}

	v3_near_zero :: proc (a: Vec3) -> bool {
		s := v3_splat(1e-8)
		r := simd.lanes_lt(simd.abs(simd.swizzle(a, 0, 1, 2, 0)), s)
		return bool(simd.reduce_and(r))
	}
}

random_vec3 :: proc() -> Vec3 {
	return Vec3{rand.float32(), rand.float32(), rand.float32()}
}

random_vec3_in_range :: proc(min, max: f32) -> Vec3 {
	return Vec3{rand.float32_range(min, max), rand.float32_range(min, max), rand.float32_range(min, max)}
}

random_in_unit_sphere :: proc() -> Vec3 {
	for {
		p := random_vec3_in_range(-1, 1)
		if v3_length2(p) >= 1 do continue
		return p
	}
}

random_unit_vector :: proc() -> Vec3 {
	return v3_normalize(random_in_unit_sphere())
}

random_in_hemisphere :: proc(normal: Vec3) -> Vec3 {
	in_unit_sphere := random_in_unit_sphere()
	if v3_dot(in_unit_sphere, normal) > 0 {
		return in_unit_sphere
	} else {
		return -in_unit_sphere
	}
}

write_color :: proc(out: io.Writer, c: Color, samples_per_pixel: f32) {
	avg_c := v3_sqrt(v3_splat(1.0 / samples_per_pixel) * c)
	c_arr := v3_to_array(avg_c)
	r := c_arr[0]
	g := c_arr[1]
	b := c_arr[2]

	fmt.wprintf(out, "%v %v %v\n", int(256 * math.clamp(r, 0, 0.999)), int(256 * math.clamp(g, 0, 0.999)), int(256 * math.clamp(b, 0, 0.999)))
}

random_in_unit_disk :: proc() -> Vec3 {
	for {
		p := Vec3{rand.float32_range(-1, 1), rand.float32_range(-1, 1), 0}
		if v3_length2(p) >= 1 { continue }
		return p
	}
}
