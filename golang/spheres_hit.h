#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct float3 { float v[3]; } __attribute__ ((aligned(16))) float3;

typedef struct Ray {
    float3  origin;
    float3  dir;
} Ray;

extern bool spheres_hit(const float * center_x, const float * center_y, const float * center_z, const float * radius, int32_t spheres_count, const struct Ray *ray, float t_min, float t_max, float * out_t, int32_t * out_hit_index);