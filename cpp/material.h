#pragma once

#include "rtweekend.h"
#include "hittable.h"

struct material
{
	enum KIND
	{
		KIND_LAMBERTIAN,
		KIND_METAL,
		KIND_DIELECTRIC,
	};

	KIND kind;
	color albedo;
	real_t fuzz;
	real_t ir;

	bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const
	{
		switch (kind)
		{
		case KIND_LAMBERTIAN:
		{
			auto scatter_direction = rec.normal + random_unit_vector();

			if (scatter_direction.near_zero())
				scatter_direction = rec.normal;

			scattered = ray(rec.p, scatter_direction);
			attenuation = albedo;
			return true;
		}
		case KIND_METAL:
		{
			vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
			scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
			attenuation = albedo;
			return (dot(scattered.direction(), rec.normal) > 0);
		}
		case KIND_DIELECTRIC:
		{
			attenuation = color{1, 1, 1};
			real_t refraction_ratio = rec.front_face ? (1.0/ir) : ir;

			vec3 unit_direction = unit_vector(r_in.direction());
			auto cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
			auto sin_theta = sqrt(1.0 - cos_theta*cos_theta);

			bool cannot_refract = refraction_ratio * sin_theta > 1.0;
			vec3 direction{};

			if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
				direction = reflect(unit_direction, rec.normal);
			else
				direction = refract(unit_direction, rec.normal, refraction_ratio);

			scattered = ray(rec.p, direction);
			return true;
		}
		}
	}

private:
	static real_t reflectance(real_t cosine, real_t ref_idx)
	{
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};

inline static material
lambertian(const color& a)
{
	material self{};
	self.kind = material::KIND_LAMBERTIAN;
	self.albedo = a;
	return self;
}

inline static material
metal(const color& a, real_t f)
{
	material self{};
	self.kind = material::KIND_METAL;
	self.albedo = a;
	self.fuzz = f;
	return self;
}

inline static material
dielectric(real_t ir)
{
	material self{};
	self.kind = material::KIND_DIELECTRIC;
	self.ir = ir;
	return self;
}
