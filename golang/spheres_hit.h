#pragma once

#include <stdint.h>
#include <stdbool.h>

struct float3 { float v[3]; } __attribute__ ((aligned(16)));

struct Ray {
    struct float3  origin;
    struct float3  dir;
};

extern bool spheres_hit(const float * center_x, const float * center_y, const float * center_z, const float * radius, int32_t spheres_count, const struct Ray *ray, float t_min, float t_max, float * out_t, int32_t * out_hit_index);