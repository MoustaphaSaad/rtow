package main

import "core:fmt"
import "core:os"
import "core:time"
import "core:bufio"
import "core:io"
import "core:math/linalg"
import "core:math"
import "core:math/rand"

hit_sphere :: proc(center: Point3, radius: f32, r: Ray) -> f32 {
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

	a := v3_length2(r.Dir)
	oc := r.Orig - center
	half_b := v3_dot(oc, r.Dir)
	c := v3_length2(oc) - radius * radius
	discriminant := half_b * half_b - a * c
	if discriminant < 0 {
		return -1
	} else {
		return (-half_b - math.sqrt(discriminant)) / a
	}
}

infinity := math.inf_f32(1)
pi :: 3.1415926535897932385

degrees_to_radians :: proc(degrees: f32) -> f32 {
	return degrees * pi / 180
}

ray_color :: proc(r: Ray, world: ^HittableList, depth: int) -> Color {
	if depth <= 0 {
		return Color{}
	}

	if rec, hit := hittable_list_hit(world, r, 0.001, infinity); hit {
		mat := world.materials[rec.material_index]
		if res, scattered := material_scatter(mat, r, rec); scattered {
			return res.attenuation * ray_color(res.scattered, world, depth - 1)
		}
		target := rec.p + rec.normal + random_in_hemisphere(rec.normal)
		return v3_splat(0.5) * ray_color(Ray{rec.p, target - rec.p}, world, depth - 1)
	}
	unit_direction := v3_normalize(r.Dir)
	t := 0.5 * (v3_to_array(unit_direction)[1] + 1.0)
	return Color{1, 1, 1} * v3_splat(1 - t) + Color{0.5, 0.7, 1} * v3_splat(t)
}

random_scene :: proc() -> (res: ^HittableList) {
	res = hittable_list_new()

	ground_material := hittable_list_add_material(res, Lambertian{Color{0.5, 0.5, 0.5}})
	hittable_list_add_sphere(res, Sphere {
		center = Point3{0, -1000, 0},
		radius = 1000,
		material_index = ground_material,
	})

	for a in -11..<11 {
		for b in -11..<11 {
			choose_mat := rand.float32()
			center := Point3{f32(a) + 0.9*rand.float32(), 0.2, f32(b) + 0.9*rand.float32()}

			if v3_length(center - Point3{4, 0.2, 0}) > 0.9 {
				sphere_material: int

				if choose_mat < 0.8 {
					albedo := random_vec3() * random_vec3()
					sphere_material = hittable_list_add_material(res, Lambertian{albedo})
					hittable_list_add_sphere(res, Sphere{
						center,
						0.2,
						sphere_material,
					})
				} else if choose_mat < 0.95 {
					albedo := rand.float32_range(0.5, 1)
					fuzz := rand.float32_range(0, 0.5)
					sphere_material = hittable_list_add_material(res, Metal{albedo, fuzz})
					hittable_list_add_sphere(res, Sphere{
						center,
						0.2,
						sphere_material,
					})
				} else {
					sphere_material = hittable_list_add_material(res, Dielectric{1.5})
					hittable_list_add_sphere(res, Sphere{
						center,
						0.2,
						sphere_material,
					})
				}
			}
		}
	}

	material1 := hittable_list_add_material(res, Dielectric{1.5})
	hittable_list_add_sphere(res, Sphere{
		Point3{0, 1, 0},
		1.0,
		material1,
	})

	material2 := hittable_list_add_material(res, Lambertian{Color{0.4, 0.2, 0.1}})
	hittable_list_add_sphere(res, Sphere{
		Point3{-4, 1, 0},
		1.0,
		material2,
	})

	material3 := hittable_list_add_material(res, Metal{Color{0.7, 0.6, 0.5}, 0})
	hittable_list_add_sphere(res, Sphere{
		Point3{4, 1, 0},
		1.0,
		material3,
	})

	return
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

	// control the randomness
	rand.set_global_seed(42)

	// Image
	aspect_ratio : f32 = 16.0 / 9.0
	image_width := 640
	image_height := int(f32(image_width) / aspect_ratio)
	samples_per_pixel := 10
	rays_count := image_width * image_height * samples_per_pixel
	max_depth := 50

	// World
	world := random_scene()

	// Camera
	lookfrom := Point3{13, 2, 3}
	lookat := Point3{0, 0, 0}
	vup := Vec3{0, 1, 0}
	dist_to_focus : f32 = 10.0
	aperture : f32 = 0.1
	cam := new_camera(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus)

	img := image_new(image_width, image_height)
	defer image_free(img)

	for j := image_height - 1; j >= 0; j -= 1 {
		for i in 0 ..< image_width {
			pixel_color := Color{0, 0, 0}
			for s in 0 ..< samples_per_pixel {
				u := (f32(i) + rand.float32()) / f32(image_width - 1)
				v := (f32(j) + rand.float32()) / f32(image_height - 1)
				r := camera_ray(cam, u, v)
				pixel_color += ray_color(r, world, max_depth)
			}
			avg_c := v3_sqrt(v3_splat(1.0 / f32(samples_per_pixel)) * pixel_color)
			ix := image_index(img, i, j)
			img.pixels[ix] = avg_c
		}
	}

	pixel_only := time.since(start)

	image_write(img, stdout)

	fmt.wprintf(stderr, "\nDone.\n")
	fmt.wprintf(stderr, "Elapsed time: %v ms\n", time.duration_milliseconds(pixel_only))
	fmt.wprintf(stderr, "Ray Per Sec: %.2f MRays/Second\n", f64(rays_count) / time.duration_seconds(pixel_only) / 1000000)
}
