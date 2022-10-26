#pragma once

#include "hittable.h"
#include "vec3.h"

class sphere: public hittable
{
public:
	sphere() {}
	sphere(point3 cen, double r)
		: center(cen),
		  radius(r)
	{}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
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
		return true;
	}

	point3 center;
	double radius;
};