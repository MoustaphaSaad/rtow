#pragma once

#include "rtweekend.h"
#include "vec3.h"

#include <vector>

struct image
{
	int width, height;
	std::vector<color> pixels;

	image(int w, int h)
	{
		width = w;
		height = h;
		pixels.resize(width * height, color{});
	}

	color& operator()(int x, int y)
	{
		return pixels[y * width + x];
	}

	const color& operator()(int x, int y) const
	{
		return pixels[y * width + x];
	}

	void write(std::ostream& out)
	{
		std::cout << "P3\n" << width << ' ' << height << "\n255\n";
		for (int y = height - 1; y >= 0; --y)
		{
			auto y_offset = y * width;
			for (int x = 0; x < width; ++x)
			{
				auto& c = pixels[y_offset + x];
				out << static_cast<int>(256 * clamp(c.x(), 0.0, 0.999)) << ' '
					<< static_cast<int>(256 * clamp(c.y(), 0.0, 0.999)) << ' '
					<< static_cast<int>(256 * clamp(c.z(), 0.0, 0.999)) << '\n';
			}
		}
	}
};


