#include "aabb.h"

void aabb_set_min_max(AABB* aabb, float x, float y, float z)
{
    aabb->min_p.v[0] = aabb->max_p.v[0] = x;
    aabb->min_p.v[1] = aabb->max_p.v[1] = y;
    aabb->min_p.v[2] = aabb->max_p.v[2] = z;
}

void aabb_combine_aabb(AABB* dest, AABB* src)
{
    for (int i = 0; i < 3; ++i)
    {
        if (src->min_p.v[i] < dest->min_p.v[i])
        {
            dest->min_p.v[i] = src->min_p.v[i];
        }

        if (dest->max_p.v[i] < src->max_p.v[i])
        {
            dest->max_p.v[i] = src->max_p.v[i];
        }
    }
}

void aabb_combine_float(AABB* dest, float* src)
{
    for (int i = 0; i < 3; ++i)
    {
        if (src[i] < dest->min_p.v[i])
        {
            dest->min_p.v[i] = src[i];
        }

        if (dest->max_p.v[i] < src[i])
        {
            dest->max_p.v[i] = src[i];
        }
    }
}

void aabb_get_center(AABB* aabb, float* out_centers)
{
    out_centers[0] = aabb->min_p.v[0] + (aabb->max_p.v[0] - aabb->min_p.v[0]) / 2.f;
    out_centers[1] = aabb->min_p.v[1] + (aabb->max_p.v[1] - aabb->min_p.v[1]) / 2.f;
    out_centers[2] = aabb->min_p.v[2] + (aabb->max_p.v[2] - aabb->min_p.v[2]) / 2.f;
}

void aabb_get_extents(AABB* aabb, float* out_extents)
{
    out_extents[0] = aabb->max_p.v[0] - aabb->min_p.v[0];
    out_extents[1] = aabb->max_p.v[1] - aabb->min_p.v[1];
    out_extents[2] = aabb->max_p.v[2] - aabb->min_p.v[2];
}

void aabb_get_half_extents(AABB* aabb, float* out_half_extents)
{
    out_half_extents[0] = (aabb->max_p.v[0] - aabb->min_p.v[0]) / 2.f;
    out_half_extents[1] = (aabb->max_p.v[1] - aabb->min_p.v[1]) / 2.f;
    out_half_extents[2] = (aabb->max_p.v[2] - aabb->min_p.v[2]) / 2.f;
}

int aabb_get_logest_axis_index(AABB* aabb)
{
    float extents[3] =
    {
        aabb->max_p.v[0] - aabb->min_p.v[0],
        aabb->max_p.v[1] - aabb->min_p.v[1],
        aabb->max_p.v[2] - aabb->min_p.v[2]
    };

    if (extents[0] > extents[1])
    {
        if (extents[0] > extents[2])
        {
            return 0;
        }
        else
        {
            return 2;
        }
    }
    else
    {
        if (extents[1] > extents[2])
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }
}

bool aabb_intersect_aabb(AABB* a, AABB* b)
{
    if (a->max_p.v[0] < b->min_p.v[0] || a->min_p.v[0] > b->max_p.v[0]) return false;
    if (a->max_p.v[1] < b->min_p.v[1] || a->min_p.v[1] > b->max_p.v[1]) return false;
    if (a->max_p.v[2] < b->min_p.v[2] || a->min_p.v[2] > b->max_p.v[2]) return false;

    return true;
}

bool aabb_contain_point(AABB* aabb, Vector3 query_point)
{
    if (query_point.v[0] < aabb->min_p.v[0] || query_point.v[0] > aabb->max_p.v[0]) return false;
    if (query_point.v[1] < aabb->min_p.v[1] || query_point.v[1] > aabb->max_p.v[1]) return false;
    if (query_point.v[2] < aabb->min_p.v[2] || query_point.v[2] > aabb->max_p.v[2]) return false;
    
    return true;
}

float aabb_distance_exterior_sq_point(AABB* aabb, Vector3 query_point)
{
    float dist_sq = 0.f;
    float temp;
    for (int i = 0; i < 3; ++i)
    {
        if (query_point.v[i] < aabb->min_p.v[i])
        {
            temp = aabb->min_p.v[i] - query_point.v[i];
            dist_sq += temp * temp;
        }
        else if (query_point.v[i] > aabb->max_p.v[i])
        {
            temp = query_point.v[i] - aabb->max_p.v[i];
            dist_sq += temp * temp;
        }
    }
    return dist_sq;
}