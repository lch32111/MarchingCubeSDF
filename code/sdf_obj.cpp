#include "sdf_obj.h"

#include <thread>
#include <stack>

#include "common.h"
#include "obj.h"
#include "geometry_algorithm.h"


static inline void grid_init(Grid* grid, ObjData::Shape& shape, SDFObjData* sod)
{
    Vector3 min_pos = vector3_setp(shape.min_positions);
    Vector3 max_pos = vector3_setp(shape.max_positions);

    Vector3 voxel_pad = vector3_set1(sod->grid_delta * sod->grid_padding);
    min_pos = vector3_sub(min_pos, voxel_pad);
    max_pos = vector3_add(max_pos, voxel_pad);

    // after setting min/max pos, change it into grid_delta to use it as padding to find another closest triangle
    voxel_pad = vector3_set1(sod->grid_delta); 

    int xm = (int)floorf(min_pos.v[0] / sod->grid_delta);
    int ym = (int)floorf(min_pos.v[1] / sod->grid_delta);
    int zm = (int)floorf(min_pos.v[2] / sod->grid_delta);

    int xp = (int)ceilf(max_pos.v[0] / sod->grid_delta);
    int yp = (int)ceilf(max_pos.v[1] / sod->grid_delta);
    int zp = (int)ceilf(max_pos.v[2] / sod->grid_delta);

    grid->min_pos[0] = xm * sod->grid_delta;
    grid->min_pos[1] = ym * sod->grid_delta;
    grid->min_pos[2] = zm * sod->grid_delta;

    grid->max_pos[0] = xp * sod->grid_delta;
    grid->max_pos[1] = yp * sod->grid_delta;
    grid->max_pos[2] = zp * sod->grid_delta;

    grid->dimensions[0] = grid->max_pos[0] - grid->min_pos[0];
    grid->dimensions[1] = grid->max_pos[1] - grid->min_pos[1];
    grid->dimensions[2] = grid->max_pos[2] - grid->min_pos[2];

    grid->nx = xp - xm;
    grid->ny = yp - ym;
    grid->nz = zp - zm;

    grid->sdfs = std::vector<float>(grid->nx * grid->ny * grid->nz, 10000000.f);
    grid->sdf_debugs.resize(grid->sdfs.size());

    Vector3 voxel_center;
    AABB voxel_aabb;
    std::vector<int> triangle_faces;
    int grid_index = 0;
    int pos_index = 0;
    uint32_t tri_inds[3];
    Vector3 ta, tb, tc;
    Vector3 tcp;
    float dist;
    float cur_sdf;
    uint32_t closest_tri_pos_index;
    Vector3 closest_tri_pos;
    Vector3 closest_tri_normal;
    Vector3 tcp_to_vc;

    for (int k = 0; k < grid->nz; ++k)
    {
        voxel_center.v[2] = grid->min_pos[2] + sod->grid_delta * k;
        voxel_aabb.min_p.v[2] = voxel_center.v[2] - sod->grid_delta * 0.5f;
        voxel_aabb.max_p.v[2] = voxel_center.v[2] + sod->grid_delta * 0.5f;
        
        for (int j = 0; j < grid->ny; ++j)
        {
            voxel_center.v[1] = grid->min_pos[1] + sod->grid_delta * j;
            voxel_aabb.min_p.v[1] = voxel_center.v[1] - sod->grid_delta * 0.5f;
            voxel_aabb.max_p.v[1] = voxel_center.v[1] + sod->grid_delta * 0.5f;

            for (int i = 0; i < grid->nx; ++i)
            {
                voxel_center.v[0] = grid->min_pos[0] + sod->grid_delta * i;
                voxel_aabb.min_p.v[0] = voxel_center.v[0] - sod->grid_delta * 0.5f;
                voxel_aabb.max_p.v[0] = voxel_center.v[0] + sod->grid_delta * 0.5f;

                grid_index = (k * grid->ny + j) * grid->nx + i;
                SDFDebug& sd = grid->sdf_debugs[grid_index];
                cur_sdf = grid->sdfs[grid_index];

                while (sd.is_set == false)
                {
                    triangle_faces.clear();
                    bvh_intersect_aabb_with_leaf(&shape, voxel_aabb, &triangle_faces);

                    if (triangle_faces.size() == 0)
                    {
                        voxel_aabb.min_p = vector3_sub(voxel_aabb.min_p, voxel_pad);
                        voxel_aabb.max_p = vector3_add(voxel_aabb.max_p, voxel_pad);
                        continue;
                    }

                    for (int face_index : triangle_faces)
                    {
                        pos_index = face_index * 3;

                        tri_inds[0] = shape.indices[pos_index] * 3;
                        tri_inds[1] = shape.indices[pos_index + 1] * 3;
                        tri_inds[2] = shape.indices[pos_index + 2] * 3;

                        ta = vector3_setp(&(shape.positions[tri_inds[0]]));
                        tb = vector3_setp(&(shape.positions[tri_inds[1]]));
                        tc = vector3_setp(&(shape.positions[tri_inds[2]]));

                        tcp = triangle_closest_point(voxel_center, ta, tb, tc);

                        dist = vector3_distance_sq(voxel_center, tcp);

                        if (dist < cur_sdf)
                        {
                            cur_sdf = dist;
                            closest_tri_pos_index = pos_index;
                            closest_tri_pos = tcp;
                        }
                    }

                    tri_inds[0] = shape.indices[closest_tri_pos_index] * 3;
                    tri_inds[1] = shape.indices[closest_tri_pos_index + 1] * 3;
                    tri_inds[2] = shape.indices[closest_tri_pos_index + 2] * 3;
                    ta = vector3_setp(&(shape.positions[tri_inds[0]]));
                    tb = vector3_setp(&(shape.positions[tri_inds[1]]));
                    tc = vector3_setp(&(shape.positions[tri_inds[2]]));

                    closest_tri_normal = vector3_cross(vector3_sub(tb, ta), vector3_sub(tc, ta));
                    tcp_to_vc = vector3_sub(voxel_center, closest_tri_pos);

                    cur_sdf = sqrtf(cur_sdf);
                    if (vector3_dot(closest_tri_normal, tcp_to_vc) > 0.f)
                    {
                        grid->sdfs[grid_index] = cur_sdf;
                    }
                    else
                    {
                        grid->sdfs[grid_index] = cur_sdf * -1.f;
                    }

                    sd.is_set = true;
                    sd.tri[0] = ta;
                    sd.tri[1] = tb;
                    sd.tri[2] = tc;
                    sd.closest_tri_pos = closest_tri_pos;
                    sd.closest_tri_normal = vector3_normalize(closest_tri_normal);
                }
            }
        }
    }
}

struct Grid2Work
{
    Grid* grid;
    ObjData::Shape* shape;
    Vector3* voxels;
    int begin;
    int end;
};
static void grid2_work(void* param)
{
    Grid2Work& work = *(Grid2Work*)param;
    Grid* grid = work.grid;
    ObjData::Shape& shape = *(work.shape);

    int grid_index = 0;
    int pos_index = 0;
    uint32_t tri_inds[3];
    Vector3 ta, tb, tc;
    float cur_sdf;
    int closest_tri_pos_index;
    Vector3 closest_tri_pos;
    Vector3 closest_tri_normal;
    Vector3 tcp_to_vc;
    Vector3 voxel_center;

    for (int grid_index = work.begin; grid_index < work.end; ++grid_index)
    {
        SDFDebug& sd = grid->sdf_debugs[grid_index];
        voxel_center = work.voxels[grid_index];

        minimum_squared_distance(&shape, voxel_center, &cur_sdf, &closest_tri_pos, &closest_tri_pos_index);

        pos_index = closest_tri_pos_index * 3;

        tri_inds[0] = shape.indices[pos_index] * 3;
        tri_inds[1] = shape.indices[pos_index + 1] * 3;
        tri_inds[2] = shape.indices[pos_index + 2] * 3;

        ta = vector3_setp(&(shape.positions[tri_inds[0]]));
        tb = vector3_setp(&(shape.positions[tri_inds[1]]));
        tc = vector3_setp(&(shape.positions[tri_inds[2]]));

        closest_tri_normal = vector3_cross(vector3_sub(tb, ta), vector3_sub(tc, ta));
        tcp_to_vc = vector3_sub(voxel_center, closest_tri_pos);
        cur_sdf = sqrtf(cur_sdf);
        if (vector3_dot(closest_tri_normal, tcp_to_vc) > 0.f)
        {
            grid->sdfs[grid_index] = cur_sdf;
        }
        else
        {
            grid->sdfs[grid_index] = cur_sdf * -1.f;
        }

        sd.is_set = true;
        sd.tri[0] = ta;
        sd.tri[1] = tb;
        sd.tri[2] = tc;
        sd.closest_tri_pos = closest_tri_pos;
        sd.closest_tri_normal = vector3_normalize(closest_tri_normal);
    }
}

static inline void grid_init2(Grid* grid, ObjData::Shape& shape, SDFObjData* sod)
{
    Vector3 min_pos = vector3_setp(shape.min_positions);
    Vector3 max_pos = vector3_setp(shape.max_positions);

    Vector3 voxel_pad = vector3_set1(sod->grid_delta * sod->grid_padding);
    min_pos = vector3_sub(min_pos, voxel_pad);
    max_pos = vector3_add(max_pos, voxel_pad);

    // after setting min/max pos, change it into grid_delta to use it as padding to find another closest triangle
    voxel_pad = vector3_set1(sod->grid_delta);

    int xm = (int)floorf(min_pos.v[0] / sod->grid_delta);
    int ym = (int)floorf(min_pos.v[1] / sod->grid_delta);
    int zm = (int)floorf(min_pos.v[2] / sod->grid_delta);

    int xp = (int)ceilf(max_pos.v[0] / sod->grid_delta);
    int yp = (int)ceilf(max_pos.v[1] / sod->grid_delta);
    int zp = (int)ceilf(max_pos.v[2] / sod->grid_delta);

    grid->min_pos[0] = xm * sod->grid_delta;
    grid->min_pos[1] = ym * sod->grid_delta;
    grid->min_pos[2] = zm * sod->grid_delta;

    grid->max_pos[0] = xp * sod->grid_delta;
    grid->max_pos[1] = yp * sod->grid_delta;
    grid->max_pos[2] = zp * sod->grid_delta;

    grid->dimensions[0] = grid->max_pos[0] - grid->min_pos[0];
    grid->dimensions[1] = grid->max_pos[1] - grid->min_pos[1];
    grid->dimensions[2] = grid->max_pos[2] - grid->min_pos[2];

    grid->nx = xp - xm;
    grid->ny = yp - ym;
    grid->nz = zp - zm;

    grid->sdfs = std::vector<float>(grid->nx * grid->ny * grid->nz, 10000000.f);
    grid->sdf_debugs.resize(grid->sdfs.size());

    Vector3 voxel_center;
    int grid_index = 0;
    /*
    int pos_index = 0;
    uint32_t tri_inds[3];
    Vector3 ta, tb, tc;
    Vector3 tcp;
    float dist;
    float cur_sdf;
    int closest_tri_pos_index;
    Vector3 closest_tri_pos;
    Vector3 closest_tri_normal;
    Vector3 tcp_to_vc;
    */

    std::vector<Vector3> voxels;
    voxels.resize(grid->sdfs.size());

    for (int k = 0; k < grid->nz; ++k)
    {
        voxel_center.v[2] = grid->min_pos[2] + sod->grid_delta * k;

        for (int j = 0; j < grid->ny; ++j)
        {
            voxel_center.v[1] = grid->min_pos[1] + sod->grid_delta * j;

            for (int i = 0; i < grid->nx; ++i)
            {
                voxel_center.v[0] = grid->min_pos[0] + sod->grid_delta * i;

                grid_index = (k * grid->ny + j) * grid->nx + i;
                voxels[grid_index] = voxel_center;

                /*
                SDFDebug& sd = grid->sdf_debugs[grid_index];

                minimum_squared_distance(&shape, voxel_center, &cur_sdf, &closest_tri_pos, &closest_tri_pos_index);

                pos_index = closest_tri_pos_index * 3;

                tri_inds[0] = shape.indices[pos_index] * 3;
                tri_inds[1] = shape.indices[pos_index + 1] * 3;
                tri_inds[2] = shape.indices[pos_index + 2] * 3;

                ta = vector3_setp(&(shape.positions[tri_inds[0]]));
                tb = vector3_setp(&(shape.positions[tri_inds[1]]));
                tc = vector3_setp(&(shape.positions[tri_inds[2]]));

                closest_tri_normal = vector3_cross(vector3_sub(tb, ta), vector3_sub(tc, ta));
                tcp_to_vc = vector3_sub(voxel_center, closest_tri_pos);
                cur_sdf = sqrtf(cur_sdf);
                if (vector3_dot(closest_tri_normal, tcp_to_vc) > 0.f)
                {
                    grid->sdfs[grid_index] = cur_sdf;
                }
                else
                {
                    grid->sdfs[grid_index] = cur_sdf * -1.f;
                }

                sd.is_set = true;
                sd.tri[0] = ta;
                sd.tri[1] = tb;
                sd.tri[2] = tc;
                sd.closest_tri_pos = closest_tri_pos;
                sd.closest_tri_normal = vector3_normalize(closest_tri_normal);
                */
            }
        }
    }

    ThreadPool tp;
    int tc = (int)tp.GetThreadCount();
    int total_task_count = (int)grid->sdfs.size();
    int each_task_count = total_task_count / tc;
    
    std::vector<Grid2Work> works(total_task_count);
    for (int i = 0; i < tc; ++i)
    {
        Grid2Work& work = works[i];
        work.begin = each_task_count * i;
        work.end = each_task_count * (i + 1);
        if (work.end > total_task_count)
            work.end = total_task_count;
        work.grid = grid;
        work.shape = &shape;
        work.voxels = voxels.data();

        tp.EnqueueJob(grid2_work, &work);
    }

    tp.Join(tp.SHUTDOWN_GRACEFULLY);
}

struct GridWork
{
    SDFObjData* sod;
    ObjData::Shape* shape;
    Grid* grid;
};

void grid_task(void* param)
{
    GridWork& work = *((GridWork*)param);
    // grid_init(work.grid, *(work.shape), work.sod);
    grid_init2(work.grid, *(work.shape), work.sod);
}

SDFObjData* sdf_obj_load(const char* path, float model_scale, float grid_delta)
{
	SDFObjData* sod = new SDFObjData();
	sod->data = obj_load(path, model_scale);
	
    sod->render_mesh_by_marching_cubes = true;
	sod->render_bounds = false;
	sod->render_grid_points = false;
	sod->render_bvh = false;
    sod->render_sdf_debug_info = false;
    sod->render_sdf_debug_triangle_normal = true;
	sod->grid_delta = grid_delta;
	sod->grid_padding = 1;
    sod->iso_value = 0.f;
    
    size_t shape_count = sod->data->shapes.size();
    sod->grids.resize(shape_count);
    
    std::vector<GridWork> works;
    works.resize(shape_count);
    
    size_t work_index = 0;
    ThreadPool tp;

    clock_t time_measure = clock();

    for (size_t si = 0; si < shape_count; ++si)
    {
        ObjData::Shape& shape = sod->data->shapes[si];

        GridWork& work = works[work_index];
        ++work_index;

        work.grid = &(sod->grids[si]);
        work.shape = &shape;
        work.sod = sod;

        tp.EnqueueJob(grid_task, &work);
    }
    
    tp.Join(ThreadPool::SHUTDOWN_GRACEFULLY);

    time_measure = clock() - time_measure;

    printf("%f seconds for calculating sdf values of %llu shapes\n", (float)time_measure / CLOCKS_PER_SEC, shape_count);

	return sod;
}

void sdf_obj_unload(SDFObjData* od)
{
	obj_unload(od->data);
	delete od;
}