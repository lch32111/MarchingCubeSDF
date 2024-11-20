#include "geometry_algorithm.h"

// Real-Time Collision Detection by Christer Ericson p141-142
Vector3 triangle_closest_point(Vector3 p, Vector3 a, Vector3 b, Vector3 c)
{
    Vector3 ab = vector3_sub(b, a);
    Vector3 ac = vector3_sub(c, a);
    Vector3 ap = vector3_sub(p, a);
    float d1 = vector3_dot(ab, ap);
    float d2 = vector3_dot(ac, ap);

    if (d1 <= 0.f && d2 <= 0.f) return a;

    Vector3 bp = vector3_sub(p, b);
    float d3 = vector3_dot(ab, bp);
    float d4 = vector3_dot(ac, bp);
    if (d3 >= 0.f && d4 <= d3) return b;

    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f)
    {
        float v = d1 / (d1 - d3);
        return vector3_add(a, vector3_mul_scalar(ab, v));
    }

    Vector3 cp = vector3_sub(p, c);
    float d5 = vector3_dot(ab, cp);
    float d6 = vector3_dot(ac, cp);
    if (d6 >= 0.f && d5 <= d6) return c;

    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f)
    {
        float  w = d2 / (d2 - d6);
        return vector3_add(a, vector3_mul_scalar(ac, w));
    }

    float va = d3 * d6 - d5 * d4;
    if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f)
    {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return vector3_add(b, vector3_mul_scalar(vector3_sub(c, b), w));
    }

    float denom = 1.f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return vector3_add(a, vector3_add(vector3_mul_scalar(ab, v), vector3_mul_scalar(ac, w)));
}

// Real-Time Collision Detection by Christer Ericson p190-194
// this function return true in the case of intersection with the false plane of the triangle
bool triangle_intersect_ray(TriangleRayIntersect& param)
{
    constexpr float ray_max_distance = 100000;
    Vector3 neg_ray_dir = vector3_mul_scalar(param.ray_dir, -1.f);

    Vector3 ab = vector3_sub(param.b, param.a);
    Vector3 ac = vector3_sub(param.c, param.a);

    Vector3 n = vector3_cross(ab, ac);
    float d = vector3_dot(neg_ray_dir, n);

    Vector3 ap = vector3_sub(param.ray_origin, param.a);
    float t = vector3_dot(ap, n);

    Vector3 e = vector3_cross(neg_ray_dir, ap);
    float v = vector3_dot(ac, e);
    if (d > 0.f)
    {
        if (v < 0.f || v > d)
            return false;
    }
    else
    {
        if (v > 0.f || v < d)
            return false;
    }
            
    float w = -vector3_dot(ab, e);
    if (d > 0.f)
    {
        if (w < 0.f || v + w > d)
            return false;
    }
    else
    {
        if (w > 0.f || v + w < d)
            return false;
    }

    float ood = 1.f / d;
    param.out_t = t * ood;
    param.out_v = v * ood;
    param.out_w = w * ood;
    param.out_u = 1.f - v - w;
    param.out_p = vector3_add(param.ray_origin, vector3_mul_scalar(param.ray_dir, param.out_t));

    return true;
}