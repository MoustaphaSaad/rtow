#pragma once

#include "ray.h"

struct material;

struct hit_record
{
	point3 p;
	vec3 normal;
	int mat_index;
	real_t t;
	bool front_face;

	inline void set_face_normal(const ray& r, const vec3& outward_normal) {
		front_face = dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};
