#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include <iostream>
#include <chrono>

real_t hit_sphere(const point3& center, real_t radius, const ray& r)
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

color ray_color(const ray& r, const hittable_list& world, int depth)
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

hittable_list random_scene()
{
	hittable_list world;

	auto ground_material = std::make_shared<material>(lambertian(color(0.5, 0.5, 0.5)));
	world.add(sphere{point3(0, -1000, 0), 1000, ground_material});

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			auto choose_mat = random_double();
			point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9)
			{
				std::shared_ptr<material> sphere_material;

				if (choose_mat < 0.8)
				{
					auto albedo = color::random() * color::random();
					sphere_material = std::make_shared<material>(lambertian(albedo));
					world.add(sphere{center, 0.2, sphere_material});
				}
				else if (choose_mat < 0.95)
				{
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = std::make_shared<material>(metal(albedo, fuzz));
					world.add(sphere{center, 0.2, sphere_material});
				}
				else
				{
					sphere_material = std::make_shared<material>(dielectric(1.5));
					world.add(sphere{center, 0.2, sphere_material});
				}
			}
		}
	}

	auto material1 = std::make_shared<material>(dielectric(1.5));
	world.add(sphere{point3(0, 1, 0), 1.0, material1});

	auto material2 = std::make_shared<material>(lambertian(color(0.4, 0.2, 0.1)));
	world.add(sphere{point3(-4, 1, 0), 1.0, material2});

	auto material3 = std::make_shared<material>(metal(color(0.7, 0.6, 0.5), 0.0));
	world.add(sphere{point3(4, 1, 0), 1.0, material3});

	return world;
}

int main()
{
	auto start = std::chrono::high_resolution_clock::now();

	// Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 640;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 10;
	const size_t rays_count = image_width * image_height * samples_per_pixel;
	const int max_depth = 50;

	// World
	auto world = random_scene();

	// Camera
	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10;
	auto aperture = 0.1;
	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	std::chrono::duration<real_t, std::milli> pixel_only{};

	for (int j = image_height - 1; j >= 0; --j)
	{
		auto end = std::chrono::high_resolution_clock::now();
		std::cerr << "\rElapsed time: " << std::chrono::duration<real_t, std::milli>(end - start).count() << "ms, ";
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
	std::cerr << "Elapsed time: " << std::chrono::duration<real_t, std::milli>(end - start).count() << "ms\n";
	std::cerr << "Pixel time: " << pixel_only.count() << "ms\n";
	std::cerr << "Ray Per Sec: " << (real_t(rays_count) / (real_t(pixel_only.count()) / 1000.0)) / 1000'000.0 << " MRays/Second\n";

	return 0;
}