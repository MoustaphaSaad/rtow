#pragma once

#include "hittable.h"
#include "vec3.h"

class sphere
{
public:
	sphere() {}
	sphere(point3 cen, real_t r, int m)
		: center(cen),
		  radius(r),
		  mat_index(m)
	{}

	bool hit(const ray& r, real_t t_min, real_t t_max, hit_record& rec) const
	{
		auto a = r.direction().length_squared();
		auto oc = r.origin() - center;
		auto half_b = dot(r.direction(), oc);
		auto c = oc.length_squared() - radius * radius;

		auto discriminant = half_b * half_b - a*c;
		if (discriminant < 0) return false;
		auto sqrtd = sqrt(discriminant);

		// find the nearest of 2 possible solutions
		auto root = (-half_b - sqrtd) / a;
		if (root < t_min || root > t_max)
		{
			root = (-half_b + sqrtd) / a;
			if (root < t_min || root > t_max)
				return false;
		}

		rec.t = root;
		rec.p = r.at(rec.t);
		auto outward_normal = (rec.p - center) / radius;
		rec.set_face_normal(r, outward_normal);
		rec.mat_index = mat_index;
		return true;
	}

	point3 center;
	real_t radius;
	int mat_index;
};