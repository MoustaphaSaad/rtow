#pragma once

#include <cmath>
#include <limits>
#include <memory>

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

using real_t = float;

const real_t infinity = std::numeric_limits<real_t>::infinity();
const real_t pi = 3.1415926535897932385;

inline real_t degrees_to_radians(real_t degrees) {
	return degrees * pi / 180.0;
}

struct random_series
{
	uint32_t state;
};

inline uint32_t xor_shift_32_rand(random_series* series)
{
	uint32_t x = series->state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 15;
	series->state = x;
	return x;
}

inline real_t random_double(random_series* series)
{
	return xor_shift_32_rand(series) / real_t(UINT32_MAX);
}

inline real_t random_double(random_series* series, real_t min, real_t max)
{
	return min + (max - min) * random_double(series);
}

inline real_t clamp(real_t x, real_t min, real_t max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

#include "ray.h"
#include "vec3.h"