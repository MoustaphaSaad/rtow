#include "color.h"
#include "vec3.h"
#include "ray.h"

#include <iostream>
#include <chrono>

color ray_color(const ray& r)
{
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

	// Camera
	auto viewport_height = 2.0;
	auto viewport_width = aspect_ratio * viewport_height;
	auto focal_length = 1.0;

	auto origin = point3{0, 0, 0};
	auto horizontal = vec3{viewport_width, 0, 0};
	auto vertical = vec3{0, viewport_height, 0};
	auto lower_left_corner = origin - horizontal/2 - vertical/2 - vec3{0, 0, focal_length};

	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	for (int j = image_height - 1; j >= 0; --j)
	{
		auto end = std::chrono::high_resolution_clock::now();
		std::cerr << "\rElapsed time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms, ";
		std::cerr << "Scan lines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i)
		{
			auto u = double(i) / (image_width - 1);
			auto v = double(j) / (image_height - 1);
			ray r{origin, (lower_left_corner + u * horizontal + v * vertical) - origin};
			auto pixel_color = ray_color(r);
			write_color(std::cout, pixel_color);
		}
	}

	std::cerr << "\nDone.\n";

	auto end = std::chrono::high_resolution_clock::now();
	std::cerr << "Elapsed time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms\n";

	return 0;
}