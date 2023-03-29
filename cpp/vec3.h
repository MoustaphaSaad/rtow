#pragma once

#include <cmath>
#include <iostream>

using std::sqrt;

#define SIMD 1

#if SIMD

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#define SHUFFLE4(V, X,Y,Z,W) _mm_shuffle_ps(V, V, _MM_SHUFFLE(W,Z,Y,X))

class vec3
{
public:
	vec3()
		: m({})
	{}

	vec3(real_t e0, real_t e1, real_t e2)
	{
		m = _mm_set_ps(0, e2, e1, e0);
	}

	vec3(real_t e)
	{
		m = _mm_set_ps1(e);
	}

	vec3(__m128 e)
	{
		m = e;
	}

	real_t x() const { return _mm_cvtss_f32(m); }
	real_t y() const { return _mm_cvtss_f32(_mm_shuffle_ps(m, m, _MM_SHUFFLE(1, 1, 1, 1))); }
	real_t z() const { return _mm_cvtss_f32(_mm_shuffle_ps(m, m, _MM_SHUFFLE(2, 2, 2, 2))); }

	vec3 operator-() const
	{
		return vec3{_mm_xor_ps(m, _mm_set1_ps(-0.0f))};
	}

	vec3& operator+=(const vec3& v)
	{
		m = _mm_add_ps(m, v.m);
		return *this;
	}

	vec3& operator*=(real_t t)
	{
		m = _mm_mul_ps(m, _mm_set_ps1(t));
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

	real_t hsum() const
	{
		auto r = _mm_mul_ps(m, _mm_set_ps(0, 1, 1, 1));
		auto shuf = SHUFFLE4(r, 1, 0, 3, 2); // we might need to flip this
		auto sums = _mm_add_ps(r, shuf);
		shuf = _mm_movehl_ps(shuf, sums);
		sums = _mm_add_ss(sums, shuf);
		return _mm_cvtss_f32(sums);
	}

	real_t length_squared() const
	{
		return vec3{_mm_mul_ps(m, m)}.hsum();
	}

	vec3 abs() const
	{
		auto minus1 = _mm_set1_epi32(-1);
		auto mask = _mm_castsi128_ps(_mm_srli_epi32(minus1, 1));
		return _mm_and_ps(mask, m);
	}

	uint32_t mask() const
	{
		return _mm_movemask_ps(m);
	}

	bool near_zero() const
	{
		const auto s = vec3{1e-8};
		auto r = vec3{SHUFFLE4(m, 0, 2, 1, 0)}.abs();
		return _mm_movemask_ps(_mm_cmplt_ps(r.m, s.m)) == 15;
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
	__m128 m;
};

using point3 = vec3;
using color = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v)
{
	return out << v.x() << ' ' << v.y() << ' ' << v.z();
}

inline vec3 operator+(const vec3& u, const vec3& v)
{
	return vec3{_mm_add_ps(u.m, v.m)};
}

inline vec3 operator-(const vec3& u, const vec3& v)
{
	return vec3{_mm_sub_ps(u.m, v.m)};
}

inline vec3 operator*(const vec3& u, const vec3& v)
{
	return vec3{_mm_mul_ps(u.m, v.m)};
}

inline vec3 operator*(real_t t, const vec3& v)
{
	return vec3{_mm_mul_ps(_mm_set_ps1(t), v.m)};
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
	return (u * v).hsum();
}

inline vec3 cross(const vec3& u, const vec3& v)
{
	auto r1 = _mm_mul_ps(SHUFFLE4(u.m, 2, 0, 1, 3), v.m);
	auto r2 = _mm_mul_ps(u.m, SHUFFLE4(v.m, 2, 0, 1, 3));
	auto r3 = _mm_sub_ps(r1, r2);
	return vec3{SHUFFLE4(r3, 2, 0, 1, 3)};
}

inline vec3 unit_vector(vec3 v)
{
	return v / v.length();
}

#else
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

#endif

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
