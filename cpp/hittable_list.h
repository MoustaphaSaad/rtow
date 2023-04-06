#pragma once

#include "hittable.h"
#include "sphere.h"
#include "spheres_hit.h"

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

	bool hit_soa(const ray& r, real_t t_min, real_t t_max, hit_record& rec) const
	{
		ispc::Ray ispc_ray{};
		ispc_ray.origin.v[0] = r.origin().x();
		ispc_ray.origin.v[1] = r.origin().y();
		ispc_ray.origin.v[2] = r.origin().z();
		ispc_ray.dir.v[0] = r.direction().x();
		ispc_ray.dir.v[1] = r.direction().y();
		ispc_ray.dir.v[2] = r.direction().z();

		float out_t{};
		int32_t out_hit_index{};

		auto res = spheres_hit(
			soa.center_x.data(),
			soa.center_y.data(),
			soa.center_z.data(),
			soa.squared_radius.data(),
			soa.inv_radius.data(),
			spheres.size(),
			ispc_ray,
			t_min,
			t_max,
			&out_t,
			&out_hit_index
		);

		if (res == false)
			return false;

		rec.t = out_t;
		rec.p = r.at(rec.t);
		vec3 center {soa.center_x[out_hit_index], soa.center_y[out_hit_index], soa.center_z[out_hit_index]};
		auto outward_normal = (rec.p - center) * soa.inv_radius[out_hit_index];
		rec.set_face_normal(r, outward_normal);
		rec.mat_index = spheres[out_hit_index].mat_index;
		return res;
	}

	std::vector<sphere> spheres;
	std::vector<material> materials;
	spheres_soa soa;
};