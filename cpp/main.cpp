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
		std::cerr << "\rElapsed time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ns, ";
		std::cerr << "Scanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i)
		{
			auto r = double(i) / (image_width - 1);
			auto g = double(j) / (image_height - 1);
			auto b = 0.25;

			int ir = static_cast<int>(255.999 * r);
			int ig = static_cast<int>(255.999 * g);
			int ib = static_cast<int>(255.999 * b);

			std::cout << ir << ' ' << ig << ' ' << ib << '\n';
		}
	}

	std::cerr << "\nDone.\n";

	auto end = std::chrono::high_resolution_clock::now();
	std::cerr << "Elapsed time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ns\n";

	return 0;
}