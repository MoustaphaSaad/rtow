#pragma once

#include "hittable.h"
#include "sphere.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

struct spheres_soa
{
	std::vector<real_t> center_x;
	std::vector<real_t> center_y;
	std::vector<real_t> center_z;
	std::vector<real_t> squared_radius;
	std::vector<real_t> inv_radius;
};

inline static size_t
_round_up(size_t num, size_t factor)
{
	if (factor == 0) return 0;
	if (num % factor == 0) return num;
	return num + factor - 1 - (num + factor - 1) % factor;
}

class hittable_list
{
public:
	hittable_list() {}
	hittable_list(const sphere& sphere) { add(sphere); }

	void clear() { spheres.clear(); }
	void add(const sphere& sphere) { spheres.push_back(sphere); }
	int add(const material& material) { materials.push_back(material); return materials.size() - 1; }

	void prepare_soa()
	{
		size_t simd_count = _round_up(spheres.size(), 4);
		soa.center_x.resize(simd_count, real_t(10000));
		soa.center_y.resize(simd_count, real_t(10000));
		soa.center_z.resize(simd_count, real_t(10000));
		soa.squared_radius.resize(simd_count, real_t(0));
		soa.inv_radius.resize(simd_count, real_t(0));
		for (size_t i = 0; i < spheres.size(); ++i)
		{
			soa.center_x[i] = spheres[i].center.x();
			soa.center_y[i] = spheres[i].center.y();
			soa.center_z[i] = spheres[i].center.z();
			soa.squared_radius[i] = spheres[i].radius * spheres[i].radius;
			soa.inv_radius[i] = real_t(1.0) / spheres[i].radius;
		}
	}

	bool hit(const ray& r, real_t t_min, real_t t_max, hit_record& rec) const
	{
		hit_record temp_rec;
		bool hit_anything = false;
		auto closest_so_far = t_max;

		for (const auto& sphere: spheres)
		{
			if (sphere.hit(r, t_min, closest_so_far, temp_rec))
			{
				hit_anything = true;
				closest_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}

		return hit_anything;
	}

	std::vector<sphere> spheres;
	std::vector<material> materials;
	spheres_soa soa;
};