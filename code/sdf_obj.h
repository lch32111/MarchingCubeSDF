#ifndef __SDF_OBJ_H__
#define __SDF_OBJ_H__

#include <vector>
#include "vector.h"

struct ObjData;

struct SDFDebug
{
    bool is_set;
    Vector3 tri[3];
    Vector3 closest_tri_pos;
    Vector3 closest_tri_normal;
};

struct Grid
{
	int nx, ny, nz;
    float min_pos[3];
    float max_pos[3];
    float dimensions[3];
    std::vector<float> sdfs;
    std::vector<SDFDebug> sdf_debugs;
};

struct SDFObjData
{
	ObjData* data;
    bool render_mesh_by_marching_cubes;
	bool render_bounds;
	bool render_grid_points;
	bool render_bvh;
    bool render_sdf_debug_info;
    bool render_sdf_debug_triangle_normal;

	float grid_delta;
	int grid_padding;
    float iso_value;

	// grids[shapes]
	std::vector<Grid> grids;
};

SDFObjData* sdf_obj_load(const char* path, float model_scale, float grid_delta);
void sdf_obj_unload(SDFObjData* od);

#endif