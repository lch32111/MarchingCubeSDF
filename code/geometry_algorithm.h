#ifndef __GEOMETRY_ALGORITHM_H__
#define __GEOMETRY_ALGORITHM_H__

#include "vector.h"

Vector3 triangle_closest_point(Vector3 p, Vector3 a, Vector3 b, Vector3 c);

struct TriangleRayIntersect
{
    Vector3 a, b, c; // triangle abc is in counter-clockwise manner.
    Vector3 ray_origin, ray_dir; // ray_dir is normalized

    // set if the intersection happens
    float out_t; // ray_origin + ray_dir * t is the hit point
    Vector3 out_p; // hit point
    float out_u, out_v, out_w;
};
bool triangle_intersect_ray(TriangleRayIntersect& param);

#endif