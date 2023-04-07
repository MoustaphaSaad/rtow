typedef float<3> float3;

struct Ray
{
	uniform float3 origin;
	uniform float3 dir;
};

float dot(float3 a, float3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float length_squared(float3 a)
{
	return dot(a, a);
}

export uniform bool spheres_hit(
	uniform const float center_x[],
	uniform const float center_y[],
	uniform const float center_z[],
	uniform const float radius[],
	uniform int spheres_count,
	uniform const Ray& ray,
	uniform float t_min,
	uniform float t_max,
	uniform float out_t[],
	uniform int out_hit_index[])
{
	float closest_so_far = t_max;
	uniform bool hit = false;
	int hit_index = 0;
	foreach (i = 0 ... spheres_count)
	{
		float a = length_squared(ray.dir);
		float3 center = {center_x[i], center_y[i], center_z[i]};
		float3 oc = ray.origin - center;
		float half_b = dot(ray.dir, oc);
		float c = length_squared(oc) - radius[i] * radius[i];

		float discriminant = half_b * half_b - a * c;
		if (discriminant < 0) continue;
		float sqrtd = sqrt(discriminant);

		// find the nearest of 2 possible solutions
		float root = (-half_b - sqrtd) / a;
		if (root < t_min || root > closest_so_far)
		{
			root = (-half_b + sqrtd) / a;
			if (root < t_min || root > closest_so_far)
				continue;
		}

		hit = true;
		closest_so_far = min(closest_so_far, root);
		hit_index = i;
	}

	if (hit == false)
		return false;

	uniform float ut = t_max;
	uniform int uhit_index = 0;
	for (uniform int i = 0; i < TARGET_WIDTH; ++i)
	{
		uniform float closest_so_far_lane = extract(closest_so_far, i);
		if (ut > closest_so_far_lane)
		{
			ut = closest_so_far_lane;
			uhit_index = extract(hit_index, i);
		}
	}

	out_t[0] = ut;
	out_hit_index[0] = uhit_index;
	return hit;
}