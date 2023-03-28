#pragma once

#include "hittable.h"
#include "sphere.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class hittable_list
{
public:
	hittable_list() {}
	hittable_list(const sphere& sphere) { add(sphere); }

	void clear() { spheres.clear(); }
	void add(const sphere& sphere) { spheres.push_back(sphere); }
	int add(const material& material) { materials.push_back(material); return materials.size() - 1; }

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
};