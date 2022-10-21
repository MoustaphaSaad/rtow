#include "color.h"
#include "vec3.h"
#include "ray.h"

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

color ray_color(const ray& r)
{
	auto t = hit_sphere(point3{0, 0, -1}, 0.5, r);
	if (t > 0) {
		auto N = unit_vector(r.at(t) - vec3{0, 0, -1});
		return 0.5 * color{N + vec3{1, 1, 1}};
	}
	auto unit_direction = unit_vector(r.direction());
	t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color{1.0, 1.0, 1.0} + t * color{0.5, 0.7, 1.0};
}

int main()
{
	auto start = std::chrono::high_resolution_clock::now();

	// Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 400;
	const int image_height = static_cast<int>(image_width / aspect_ratio);

	// Camera
	auto viewport_height = 2.0;
	auto viewport_width = aspect_ratio * viewport_height;
	auto focal_length = 1.0;

	auto origin = point3{0, 0, 0};
	auto horizontal = vec3{viewport_width, 0, 0};
	auto vertical = vec3{0, viewport_height, 0};
	auto lower_left_corner = origin - horizontal/2 - vertical/2 - vec3{0, 0, focal_length};

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
			auto u = double(i) / (image_width - 1);
			auto v = double(j) / (image_height - 1);
			ray r{origin, (lower_left_corner + u * horizontal + v * vertical) - origin};
			auto pixel_color = ray_color(r);
			auto end = std::chrono::high_resolution_clock::now();
			pixel_only += end - start;
			write_color(std::cout, pixel_color);
		}
	}

	std::cerr << "\nDone.\n";

	auto end = std::chrono::high_resolution_clock::now();
	std::cerr << "Elapsed time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms\n";
	std::cerr << "Pixel time: " << pixel_only.count() << "ms\n";

	return 0;
}