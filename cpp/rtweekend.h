#pragma once

#include <cmath>
#include <limits>
#include <memory>

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

using real_t = double;

const real_t infinity = std::numeric_limits<real_t>::infinity();
const real_t pi = 3.1415926535897932385;

inline real_t degrees_to_radians(real_t degrees) {
	return degrees * pi / 180.0;
}

inline real_t random_double()
{
	return rand() / (RAND_MAX + 1.0);
}

inline real_t random_double(real_t min, real_t max)
{
	return min + (max - min) * random_double();
}

inline real_t clamp(real_t x, real_t min, real_t max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

#include "ray.h"
#include "vec3.h"