#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "image.h"

#include <TaskScheduler.h>

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

struct raytrace_stat
{
	size_t ray_count;
	size_t bounces;
};

color ray_color(random_series* series, const ray& r, const hittable_list& world, int depth, raytrace_stat& stat)
{
	hit_record rec;

	if (depth <= 0)
		return color{};

	if (world.hit_soa(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		auto& mat = world.materials[rec.mat_index];
		if (mat.scatter(series, r, rec, attenuation, scattered))
		{
			++stat.bounces;
			return attenuation * ray_color(series, scattered, world, depth - 1, stat);
		}
		return color{};
	}
	auto unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color{1.0, 1.0, 1.0} + t * color{0.5, 0.7, 1.0};
}

hittable_list random_scene(random_series* series)
{
	hittable_list world;

	auto ground_material = world.add(lambertian(color(0.5, 0.5, 0.5)));
	world.add(sphere{point3(0, -1000, 0), 1000, ground_material});

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			auto choose_mat = random_double(series);
			point3 center(a + 0.9*random_double(series), 0.2, b + 0.9*random_double(series));

			if ((center - point3(4, 0.2, 0)).length() > 0.9)
			{
				int sphere_material;

				if (choose_mat < 0.8)
				{
					auto albedo = color::random(series) * color::random(series);
					sphere_material = world.add(lambertian(albedo));
					world.add(sphere{center, 0.2, sphere_material});
				}
				else if (choose_mat < 0.95)
				{
					auto albedo = color::random(series, 0.5, 1);
					auto fuzz = random_double(series, 0, 0.5);
					sphere_material = world.add(metal(albedo, fuzz));
					world.add(sphere{center, 0.2, sphere_material});
				}
				else
				{
					sphere_material = world.add(dielectric(1.5));
					world.add(sphere{center, 0.2, sphere_material});
				}
			}
		}
	}

	auto material1 = world.add(dielectric(1.5));
	world.add(sphere{point3(0, 1, 0), 1.0, material1});

	auto material2 = world.add(lambertian(color(0.4, 0.2, 0.1)));
	world.add(sphere{point3(-4, 1, 0), 1.0, material2});

	auto material3 = world.add(metal(color(0.7, 0.6, 0.5), 0.0));
	world.add(sphere{point3(4, 1, 0), 1.0, material3});

	world.prepare_soa();

	return world;
}

struct ImageTile
{
	int startX, startY;
	int endX, endY;
};

struct RaytraceTask: public enki::ITaskSet
{
	image* img;
	camera* cam;
	hittable_list* world;
	int samples_per_pixel;
	int max_depth;
	std::vector<ImageTile> tasks;
	std::vector<random_series> random_series;
	std::vector<raytrace_stat> stats;

	void ExecuteRange(enki::TaskSetPartition range, uint32_t thread_ix) override
	{
		auto series = &random_series[thread_ix];
		auto& stat = stats[thread_ix];
		for (uint32_t r = range.start; r < range.end; ++r)
		{
			auto& tile = tasks[r];
			for (int j = tile.startY; j < tile.endY; ++j)
			{
				for (int i = tile.startX; i < tile.endX; ++i)
				{
					color pixel_color{0, 0, 0};
					for (int s = 0; s < samples_per_pixel; ++s)
					{
						auto u = (i + random_double(series)) / (img->width - 1);
						auto v = (j + random_double(series)) / (img->height - 1);
						auto r = cam->get_ray(series, u, v);
						pixel_color += ray_color(series, r, *world, max_depth, stat);
						++stat.ray_count;
					}

					auto scale = 1.0 / samples_per_pixel;
					(*img)(i, j) = sqrt(pixel_color * scale);
				}
			}
		}
	}
};

int main()
{
	enki::TaskScheduler ts;
	ts.Initialize();

	auto start = std::chrono::high_resolution_clock::now();

	// Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 640;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 10;
	const int max_depth = 50;

	// World
	random_series global_random_series{42};
	auto world = random_scene(&global_random_series);

	// Camera
	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10;
	auto aperture = 0.1;
	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

	image img{image_width, image_height};

	int tileSizeX = 16;
	int tileSizeY = 16;
	int tileCountX = 1 + ((img.width - 1) / tileSizeX);
	int tileCountY = 1 + ((img.height - 1) / tileSizeY);

	int tileTotalCount = tileCountX * tileCountY;

	RaytraceTask job{};
	job.img = &img;
	job.cam = &cam;
	job.world = &world;
	job.samples_per_pixel = samples_per_pixel;
	job.max_depth = max_depth;

	for (int x = 0; x < tileCountX; ++x)
	{
		int startX = x * tileSizeX;
		int endX = startX + tileSizeX;
		if (endX > img.width) { endX = img.width; }

		for (int y = 0; y < tileCountY; ++y)
		{
			int startY = y * tileSizeY;
			int endY = startY + tileSizeY;
			if (endY > img.height) { endY = img.height; }

			ImageTile tile{};
			tile.startX = startX;
			tile.endX = endX;
			tile.startY = startY;
			tile.endY = endY;
			job.tasks.push_back(tile);
			job.random_series.push_back(random_series{xor_shift_32_rand(&global_random_series)});
			job.stats.push_back(raytrace_stat{});
		}
	}

	job.m_SetSize = job.tasks.size();
	ts.AddTaskSetToPipe(&job);
	ts.WaitforTask(&job);

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<real_t, std::milli> pixel_only = end - start;

	raytrace_stat global_stat{};
	for (auto& s: job.stats)
	{
		global_stat.ray_count += s.ray_count;
		global_stat.bounces += s.bounces;
	}

	img.write(std::cout);
	std::cerr << "\nDone.\n";

	std::cerr << "Elapsed time: " << std::chrono::duration<real_t, std::milli>(end - start).count() << "ms\n";
	std::cerr << "Total Rays: " << real_t(global_stat.ray_count) / real_t(1000'000.0) << " MRays, Bounces: " << real_t(global_stat.bounces) / real_t(1000'000.0) << " MRays\n";
	auto rays_count = global_stat.ray_count + global_stat.bounces;
	std::cerr << "Ray Per Sec: " << (real_t(rays_count) / (real_t(pixel_only.count()) / 1000.0)) / 1000'000.0 << " MRays/Second\n";

	return 0;
}