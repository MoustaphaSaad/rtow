#include "color.h"
#include "vec3.h"

#include <iostream>
#include <chrono>

int main()
{
	auto start = std::chrono::high_resolution_clock::now();

	const int image_width = 256;
	const int image_height = 256;

	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	for (int j = image_height - 1; j >= 0; --j)
	{
		auto end = std::chrono::high_resolution_clock::now();
		std::cerr << "\rElapsed time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms, ";
		std::cerr << "Scanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i)
		{
			color pixel_color{
				double(i) / (image_width - 1),
				double(j) / (image_height - 1),
				0.25
			};
			write_color(std::cout, pixel_color);
		}
	}

	std::cerr << "\nDone.\n";

	auto end = std::chrono::high_resolution_clock::now();
	std::cerr << "Elapsed time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms\n";

	return 0;
}