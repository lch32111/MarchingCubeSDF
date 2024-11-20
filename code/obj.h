#ifndef __OBJ_H__
#define __OBJ_H__

#include "aabb.h"
#include "vector.h"
#include <vector>

struct BVH
{
	AABB aabb;
	float center[3];
	int face_index;
	int left;
	int right;
};

struct ObjData
{
	struct Shape
	{
		std::vector<float> positions;
		std::vector<float> normals;
		std::vector<uint32_t> indices;

		float min_positions[3];
		float max_positions[3];

		std::vector<BVH> bvhs;
		int bvh_max_depth;
	};

	std::vector<Shape> shapes;
};

ObjData* obj_load(const char* path, float model_scale = 1.f);
void obj_unload(ObjData* od);

void bvh_intersect_aabb_with_leaf(ObjData::Shape* shape, AABB aabb, std::vector<int>* out_face_indices);
void minimum_squared_distance(ObjData::Shape* shape, Vector3 query_point, float* out_squared_distance, Vector3* out_closest_point, int* out_face_index);

#endif