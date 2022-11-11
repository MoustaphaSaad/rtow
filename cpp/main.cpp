#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include <iostream>
#include <chrono>

double hit_sphere(const point3& center, double radius, const ray& r)
{
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

	auto a = r.direction().length_squared();
	auto oc = r.origin() - center;
	auto half_b = dot(r.direction(), oc);
	auto c = oc.length_squared() - radius * radius;

	auto discriminant = half_b * half_b - a*c;
	if (discriminant < 0)
	{
		return -1;
	}
	else
	{
		return (-half_b - sqrt(discriminant)) / (a);
	}
}

color ray_color(const ray& r, const hittable& world, int depth)
{
	hit_record rec;

	if (depth <= 0)
		return color{};

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color{};
	}
	auto unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color{1.0, 1.0, 1.0} + t * color{0.5, 0.7, 1.0};
}

int main()
{
	auto start = std::chrono::high_resolution_clock::now();

	// Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 400;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 100;
	const size_t rays_count = image_width * image_height * samples_per_pixel;
	const int max_depth = 50;

	// World
	hittable_list world;

	auto material_ground = make_shared<lambertian>(color{0.8, 0.8, 0.0});
	auto material_center = make_shared<lambertian>(color{0.7, 0.3, 0.3});
	auto material_left = make_shared<metal>(color{0.8, 0.8, 0.8}, 0.3);
	auto material_right = make_shared<metal>(color{0.8, 0.6, 0.2}, 1);

	world.add(make_shared<sphere>(point3{0, -100.5, -1}, 100, material_ground));
	world.add(make_shared<sphere>(point3{0, 0, -1}, 0.5, material_center));
	world.add(make_shared<sphere>(point3{-1, 0, -1}, 0.5, material_left));
	world.add(make_shared<sphere>(point3{1, 0, -1}, 0.5, material_right));

	// Camera
	camera cam;

	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	std::chrono::duration<double, std::milli> pixel_only{};

	for (int j = image_height - 1; j >= 0; --j)
	{
		auto end = std::chrono::high_resolution_clock::now();
		std::cerr << "\rElapsed time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms, ";
		std::cerr << "Scan lines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i)
		{
			auto start = std::chrono::high_resolution_clock::now();
			color pixel_color{0, 0, 0};
			for (int s = 0; s < samples_per_pixel; ++s)
			{
				auto u = (i + random_double()) / (image_width - 1);
				auto v = (j + random_double()) / (image_height - 1);
				auto r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
			auto end = std::chrono::high_resolution_clock::now();
			pixel_only += end - start;
			write_color(std::cout, pixel_color, samples_per_pixel);
		}
	}

	std::cerr << "\nDone.\n";

	auto end = std::chrono::high_resolution_clock::now();
	std::cerr << "Elapsed time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms\n";
	std::cerr << "Pixel time: " << pixel_only.count() << "ms\n";
	std::cerr << "Ray Per Sec: " << (double(rays_count) / (double(pixel_only.count()) / 1000.0)) / 1000'000.0 << " MRays/Second\n";

	return 0;
}