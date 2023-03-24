#pragma once

#include <cmath>
#include <iostream>

using std::sqrt;

class vec3
{
public:
	vec3()
		: e{0, 0, 0}
	{}

	vec3(real_t e0, real_t e1, real_t e2)
		: e{e0, e1, e2}
	{}

	real_t x() const { return e[0]; }
	real_t y() const { return e[1]; }
	real_t z() const { return e[2]; }

	vec3 operator-() const { return vec3{-e[0], -e[1], -e[2]}; }
	real_t operator[](int i) const { return e[i]; }
	real_t& operator[](int i) { return e[i]; }

	vec3& operator+=(const vec3& v)
	{
		e[0] += v.e[0];
		e[1] += v.e[1];
		e[2] += v.e[2];
		return *this;
	}

	vec3& operator*=(real_t t)
	{
		e[0] *= t;
		e[1] *= t;
		e[2] *= t;
		return *this;
	}

	vec3& operator/=(real_t t)
	{
		return *this *= 1/t;
	}

	real_t length() const
	{
		return sqrt(length_squared());
	}

	real_t length_squared() const
	{
		return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
	}

	bool near_zero() const
	{
		const auto s = 1e-8;
		return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
	}

	inline static vec3 random()
	{
		return vec3{random_double(), random_double(), random_double()};
	}

	inline static vec3 random(real_t min, real_t max)
	{
		return vec3{random_double(min, max), random_double(min, max), random_double(min, max)};
	}

public:
	real_t e[3];
};

using point3 = vec3;
using color = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v)
{
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v)
{
	return vec3{u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]};
}

inline vec3 operator-(const vec3& u, const vec3& v)
{
	return vec3{u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]};
}

inline vec3 operator*(const vec3& u, const vec3& v)
{
	return vec3{u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]};
}

inline vec3 operator*(real_t t, const vec3& v)
{
	return vec3{t * v.e[0], t * v.e[1], t * v.e[2]};
}

inline vec3 operator*(const vec3& v, real_t t)
{
	return t * v;
}

inline vec3 operator/(vec3 v, real_t t)
{
	return (1/t) * v;
}

inline real_t dot(const vec3& u, const vec3& v)
{
	return (
		u.e[0] * v.e[0] +
		u.e[1] * v.e[1] +
		u.e[2] * v.e[2]
	);
}

inline vec3 cross(const vec3& u, const vec3& v)
{
	return vec3{
		u.e[1] * v.e[2] - u.e[2] * v.e[1],
		u.e[2] * v.e[0] - u.e[0] * v.e[2],
		u.e[0] * v.e[1] - u.e[1] * v.e[0]
	};
}

inline vec3 unit_vector(vec3 v)
{
	return v / v.length();
}

vec3 random_in_unit_sphere()
{
	while (true)
	{
		auto p = vec3::random(-1, 1);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

vec3 random_unit_vector()
{
	return unit_vector(random_in_unit_sphere());
}

// old ray tracing book
vec3 random_in_hemisphere(const vec3& normal)
{
	auto in_unit_sphere = random_in_unit_sphere();
	if (dot(in_unit_sphere, normal) > 0.0)
		return in_unit_sphere;
	else
		return -in_unit_sphere;
}

vec3 reflect(const vec3& v, const vec3& n)
{
	return v - 2 * dot(v, n) * n;
}

vec3 refract(const vec3& uv, const vec3& n, real_t etai_over_etat)
{
	auto cos_theta = fmin(dot(-uv, n), 1.0);
	auto r_out_perp = etai_over_etat * (uv + cos_theta*n);
	auto r_out_parallel = -sqrt(fabs(1 - r_out_perp.length_squared())) * n;
	return r_out_perp + r_out_parallel;
}

vec3 random_in_unit_disk()
{
	while (true)
	{
		auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}
