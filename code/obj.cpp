#include "obj.h"

#include <string>
#include <assert.h>

#include "common.h"
#include "geometry_algorithm.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


struct Shape
{
	std::string name;
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> uvs;

	std::vector<uint32_t> pos_indices;
	std::vector<uint32_t> normal_indices;
	std::vector<uint32_t> uv_indices;
};

/*
static inline void push_obj_data(ObjData::Frame& frame, const tinyobj::ObjReader& reader)
{
	const tinyobj::attrib_t& attrib = reader.GetAttrib();
	const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
	const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();
 
	frame.shapes.resize(shapes.size());
	for (size_t s = 0; s < shapes.size(); ++s)
	{
		const tinyobj::shape_t& shape = shapes[s];
		size_t index_offset = 0;

		ObjData::Shape& obj_shape = frame.shapes[s];
		obj_shape.positions.reserve(shape.mesh.num_face_vertices.size() * 3);
		obj_shape.normals.reserve(shape.mesh.num_face_vertices.size() * 3);
		obj_shape.indices.reserve(shape.mesh.num_face_vertices.size() * 3);

		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f)
		{
			size_t fv = shape.mesh.num_face_vertices[f];
			assert(fv == 3);

			for (size_t v = 0; v < fv; ++v)
			{
				obj_shape.indices.push_back((uint32_t)obj_shape.indices.size());

				tinyobj::index_t idx = shape.mesh.indices[v + index_offset];

				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				obj_shape.positions.push_back(vx);
				obj_shape.positions.push_back(vy);
				obj_shape.positions.push_back(vz);

				tinyobj::real_t nx = attrib.normals[3 * idx.normal_index];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
				obj_shape.normals.push_back(nx);
				obj_shape.normals.push_back(ny);
				obj_shape.normals.push_back(nz);
			}

			index_offset += fv;
		}
	}
}
*/

// from godot/sort_array.h
template<class T, class Comparator>
class SortArray
{
public:
	Comparator compare;

	inline int bitlog(int n)
	{
		int k;
		for (k = 0; n != 1; n >>= 1)
			++k;

		return k;
	}

	const T& median_of_3(const T& a, const T& b, const T& c)
	{
		if (compare(a, b))
		{
			if (compare(b, c))
				return b;
			else if (compare(a, c))
				return c;
			else
				return a;
		}
		else if (compare(a, c))
		{
			return a;
		}
		else if (compare(b, c))
		{
			return c;
		}
		else
		{
			return b;
		}
	}

	inline int partitioner(int first, int last, T pivot, T* arr)
	{
		const int unmodified_first = first;
		const int unmodified_last = last;

		while (true)
		{
			while (compare(arr[first], pivot))
			{
				++first;
			}
			--last;

			while (compare(pivot, arr[last]))
			{
				--last;
			}

			if (!(first < last))
			{
				return first;
			}

			T temp = arr[first];
			arr[first] = arr[last];
			arr[last] = temp;

			++first;
		}
	}

	inline void unguarded_linear_insert(int last, T value, T* p)
	{
		int next = last - 1;
		while (compare(value, p[next]))
		{
			p[last] = p[next];
			last = next;
			--next;
		}
		p[last] = value;
	}

	inline void linear_insert(int first, int last, T* p)
	{
		T val = p[last];
		if (compare(val, p[first]))
		{
			for (int i = last; i > first; --i)
			{
				p[i] = p[i - 1];
			}

			p[first] = val;
		}
		else
		{
			unguarded_linear_insert(last, val, p);
		}
	}

	inline void insertion_sort(int first, int last, T* p)
	{
		if (first == last)
			return;

		for (int i = first + 1; i != last; ++i)
		{
			linear_insert(first, i, p);
		}
	}

	inline void introselect(int first, int nth, int last, T* p, int max_depth)
	{
		while (last - first > 3)
		{
			if (max_depth == 0)
			{
				return;
			}

			int cut = partitioner(first, last,
				median_of_3
				(
					p[first],
					p[first + (last - first) / 2],
					p[last - 1]
				),
				p
			);

			if (cut <= nth)
			{
				first = cut;
			}
			else
			{
				last = cut;
			}
		}

		insertion_sort(first, last, p);
	}

	
	inline void nth_element(int first, int last, int nth, T* p)
	{
		if (first == last || last == nth)
		{
			return;
		}
		introselect(first, nth, last, p, bitlog(last - first));
	}
};

template<int axis>
struct BVHCompare
{
	bool operator()(const BVH* left, const BVH* right)
	{
		return left->center[axis] < right->center[axis];
	}
};

// from godot/triangle_mesh.cpp
static inline int create_bvh(BVH* p_bvh, BVH** p_bb, int p_from, int p_size, int p_depth, int& r_max_depth, int& r_max_alloc)
{
	if (p_depth > r_max_depth)
		r_max_depth = p_depth;

	if (p_size == 1)
	{
		return (int)(p_bb[p_from] - p_bvh);
	}
	else if (p_size == 0)
	{
		return -1;
	}

	AABB aabb;
	aabb = p_bb[p_from]->aabb;
	for (int i = 1; i < p_size; ++i)
	{
		aabb_combine_aabb(&aabb, &(p_bb[p_from + i]->aabb));
	}

	int li = aabb_get_logest_axis_index(&aabb);
	
	switch (li)
	{
	case 0:
		SortArray<BVH*, BVHCompare<0>> sort_x;
		sort_x.nth_element(0, p_size, p_size / 2, &p_bb[p_from]);
		break;
	case 1:
		SortArray<BVH*, BVHCompare<1>> sort_y;
		sort_y.nth_element(0, p_size, p_size / 2, &p_bb[p_from]);
		break;
	case 2:
		SortArray<BVH*, BVHCompare<2>> sort_z;
		sort_z.nth_element(0, p_size, p_size / 2, &p_bb[p_from]);
		break;
	}

	int left = create_bvh(p_bvh, p_bb, p_from, p_size / 2, p_depth + 1, r_max_depth, r_max_alloc);
	int right = create_bvh(p_bvh, p_bb, p_from + p_size / 2, p_size -  p_size / 2, p_depth + 1, r_max_depth, r_max_alloc);

	int index = r_max_alloc++;
	BVH* new_bvh = &(p_bvh[index]);
	new_bvh->aabb = aabb;
	aabb_get_center(&aabb, new_bvh->center);
	new_bvh->face_index = -1;
	new_bvh->left = left;
	new_bvh->right = right;

	return index;
}
/*
static inline void push_obj_data(ObjData::Frame& frame, const std::vector<Shape>& shapes)
{
	std::vector<ObjData::Shape>& od_shapes = frame.shapes;
	od_shapes.resize(shapes.size());

	std::vector<BVH*> bvh_ps;

	for (size_t si = 0; si < od_shapes.size(); ++si)
	{
		const Shape& shape = shapes[si];
		ObjData::Shape& od_shape = od_shapes[si];

		od_shape.min_positions[0] = od_shape.min_positions[1] = od_shape.min_positions[2] = FLT_MAX;
		od_shape.max_positions[0] = od_shape.max_positions[1] = od_shape.max_positions[2] = -FLT_MAX;
		
		od_shape.positions.resize(shape.positions.size());
		assert(shape.positions.size() % 3 == 0);
		for (size_t pi = 0; pi < shape.positions.size(); pi += 3)
		{
			od_shape.positions[pi] = shape.positions[pi];
			od_shape.positions[pi + 1] = shape.positions[pi + 1];
			od_shape.positions[pi + 2] = shape.positions[pi + 2];

			for (size_t pii = 0; pii < 3; ++pii)
			{
				if (shape.positions[pi + pii] < od_shape.min_positions[pii])
				{
					od_shape.min_positions[pii] = shape.positions[pi + pii];
				}

				if (od_shape.max_positions[pii] < shape.positions[pi + pii])
				{
					od_shape.max_positions[pii] = shape.positions[pi + pii];
				}
			}
		}

		od_shape.indices.resize(shape.pos_indices.size());
		memcpy(od_shape.indices.data(), shape.pos_indices.data(), sizeof(uint32_t) * shape.pos_indices.size());

		od_shape.normals.resize(shape.positions.size());

		od_shape.bvhs.resize(shape.pos_indices.size());
		for (size_t ni = 0; ni < shape.pos_indices.size(); ni += 3)
		{
			uint32_t vi[3] = { shape.pos_indices[ni] * 3, shape.pos_indices[ni + 1] * 3, shape.pos_indices[ni + 2] * 3 };

            Vector3 p0 = vector3_setp(&(shape.positions[vi[0]]));
            Vector3 p1 = vector3_setp(&(shape.positions[vi[1]]));
            Vector3 p2 = vector3_setp(&(shape.positions[vi[2]]));
            Vector3 normal = vector3_normalize(vector3_cross(vector3_sub(p1, p0), vector3_sub(p2, p0)));

			for (int i = 0; i < 3; ++i)
			{
				od_shape.normals[vi[i]] += normal.v[0];
				od_shape.normals[vi[i] + 1] += normal.v[1];
				od_shape.normals[vi[i] + 2] += normal.v[2];
			}

			size_t fi = ni / 3;
			BVH& bvh = od_shape.bvhs[fi];
			AABB& aabb = bvh.aabb;
			aabb_set_min_max(&aabb, p0.v[0], p0.v[1], p0.v[2]);
			aabb_combine_float(&aabb, p1.v);
			aabb_combine_float(&aabb, p2.v);
			bvh.face_index = (int)fi;
			bvh.left = -1;
			bvh.right = -1;
			aabb_get_center(&aabb, bvh.center);
		}

		for (size_t ni = 0; ni < od_shape.normals.size(); ni += 3)
		{
			float n[3] = { od_shape.normals[ni], od_shape.normals[ni + 1], od_shape.normals[ni + 2] };

			float inv_len = n[0] * n[0] + n[1] * n[1] + n[2] * n[2];
			if (inv_len > 0.f)
			{
				inv_len = 1.f / sqrtf(inv_len);

				od_shape.normals[ni] *= inv_len;
				od_shape.normals[ni + 1] *= inv_len;
				od_shape.normals[ni + 2] *= inv_len;
			}
		}

		int face_count = (int)shape.pos_indices.size() / 3;
		int max_alloc = face_count;
		od_shape.bvh_max_depth = 0;
		bvh_ps.resize(face_count);
		for (size_t fi = 0; fi < face_count; ++fi)
		{
			bvh_ps[fi] = &(od_shape.bvhs[fi]);
		}
		
		create_bvh(od_shape.bvhs.data(), bvh_ps.data(), 0, face_count, 1, od_shape.bvh_max_depth, max_alloc);
		od_shape.bvhs.resize(max_alloc);
	}
}*/

ObjData* obj_load(const char* path, float model_scale)
{
    ObjData* od = new ObjData();
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    const char* mtl_base_dir = "";
    
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path);
    if (!warn.empty())
        printf("%s\n", warn.c_str());

    if (!err.empty())
        printf("%s\n", err.c_str());
    assert(ret == true);

    for (size_t ai = 0; ai < attrib.vertices.size(); ++ai)
    {
        attrib.vertices[ai] = attrib.vertices[ai] * model_scale;
    }

    size_t shape_count = shapes.size();
    od->shapes.resize(shape_count);

    std::vector<BVH*> bvh_ps;

    for (size_t si = 0; si < shapes.size(); ++si)
    {
        tinyobj::shape_t& src_shape = shapes[si];
        ObjData::Shape& dest_shape = od->shapes[si];

        tinyobj::mesh_t& mesh = src_shape.mesh;
        std::vector<tinyobj::index_t>& mesh_indices = mesh.indices;

        dest_shape.positions.resize(attrib.vertices.size());
        dest_shape.normals.resize(attrib.vertices.size());
        dest_shape.indices.resize(mesh_indices.size());
        dest_shape.bvhs.resize(mesh_indices.size()); // will shirink at the end.

        dest_shape.min_positions[0] = dest_shape.min_positions[1] = dest_shape.min_positions[2] = FLT_MAX;
        dest_shape.max_positions[0] = dest_shape.max_positions[1] = dest_shape.max_positions[2] = -FLT_MAX;
        
        assert(attrib.vertices.size() % 3 == 0);
        // This is a duplicate process for multiple shapes
        for (size_t pi = 0; pi < attrib.vertices.size(); pi += 3)
        {
            dest_shape.positions[pi] = attrib.vertices[pi];
            dest_shape.positions[pi + 1] = attrib.vertices[pi + 1];
            dest_shape.positions[pi + 2] = attrib.vertices[pi + 2];

            for (size_t pii = 0; pii < 3; ++pii)
            {
                if (attrib.vertices[pi + pii] < dest_shape.min_positions[pii])
                {
                    dest_shape.min_positions[pii] = attrib.vertices[pi + pii];
                }

                if (dest_shape.max_positions[pii] < attrib.vertices[pi + pii])
                {
                    dest_shape.max_positions[pii] = attrib.vertices[pi + pii];
                }
            }
        }

        // evaluate normals, copy indices, create bvh for each face.
        assert(mesh_indices.size() % 3 == 0);
        for (size_t ii = 0; ii < mesh_indices.size(); ii += 3)
        {
            uint32_t vi[3] = 
            { 
                (uint32_t)mesh_indices[ii].vertex_index * 3 ,
                (uint32_t)mesh_indices[ii + 1].vertex_index * 3,
                (uint32_t)mesh_indices[ii + 2].vertex_index * 3
            };

            Vector3 p0 = vector3_setp(&attrib.vertices[vi[0]]);
            Vector3 p1 = vector3_setp(&attrib.vertices[vi[1]]);
            Vector3 p2 = vector3_setp(&attrib.vertices[vi[2]]);
            Vector3 normal = vector3_normalize(vector3_cross(vector3_sub(p1, p0), vector3_sub(p2, p0)));

            for (int ni = 0; ni < 3; ++ni)
            {
                dest_shape.normals[vi[ni]] += normal.v[0];
                dest_shape.normals[vi[ni] + 1] += normal.v[1];
                dest_shape.normals[vi[ni] + 2] += normal.v[2];
            }
            
            dest_shape.indices[ii] = mesh_indices[ii].vertex_index;
            dest_shape.indices[ii + 1] = mesh_indices[ii + 1].vertex_index;
            dest_shape.indices[ii + 2] = mesh_indices[ii + 2].vertex_index;

            int face_index = (int)ii / 3;
            BVH& bvh = dest_shape.bvhs[face_index];
            AABB& aabb = bvh.aabb;
            aabb_set_min_max(&aabb, p0.v[0], p0.v[1], p0.v[2]);
            aabb_combine_float(&aabb, p1.v);
            aabb_combine_float(&aabb, p2.v);
            bvh.face_index = face_index;
            bvh.left = -1;
            bvh.right = -1;
            aabb_get_center(&aabb, bvh.center);
        }

        // evalute mesh unit normal
        for (size_t ni = 0; ni < dest_shape.normals.size(); ni += 3)
        {
            float n[3] = { dest_shape.normals[ni], dest_shape.normals[ni + 1], dest_shape.normals[ni + 2] };

            float inv_len = n[0] * n[0] + n[1] * n[1] + n[2] * n[2];
            if (inv_len != 0.f)
            {
                inv_len = 1.f / sqrtf(inv_len);

                dest_shape.normals[ni] *= inv_len;
                dest_shape.normals[ni + 1] *= inv_len;
                dest_shape.normals[ni + 2] *= inv_len;
            }
        }

        // preparation for creating bvhs
        int face_count = (int)mesh_indices.size() / 3;
        int max_alloc = face_count;
        dest_shape.bvh_max_depth = 0;
        bvh_ps.resize(face_count);
        for (int fi = 0; fi < face_count; ++fi)
        {
            bvh_ps[fi] = &(dest_shape.bvhs[fi]);
        }
        create_bvh(dest_shape.bvhs.data(), bvh_ps.data(), 0, face_count, 1, dest_shape.bvh_max_depth, max_alloc);
        dest_shape.bvhs.resize(max_alloc); // shrink now
    }

	return od;
}

void obj_unload(ObjData* od)
{
	delete od;
}

static inline bool is_bvh_leaf(BVH* bvh)
{
    return (bvh->left == -1 && bvh->right == -1);
}

void bvh_intersect_aabb_with_leaf(ObjData::Shape* shape, AABB aabb, std::vector<int>* out_face_indices)
{
    int* stack = (int*)ALLOCA(sizeof(int) * shape->bvh_max_depth);
    assert(stack != NULL);

    int stack_index = 0;
    BVH* bvhptr = shape->bvhs.data();
    BVH* bvh = NULL;
    
    stack[stack_index] = (int)shape->bvhs.size() - 1;
    ++stack_index;
    while (true)
    {
        if (stack_index <= 0)
            break;

        int bvh_index = stack[stack_index - 1];
        --stack_index;

        bvh = &(bvhptr[bvh_index]);

        if (is_bvh_leaf(bvh) == true)
        {
            (*out_face_indices).push_back(bvh->face_index);
        }
        else
        {
            if (aabb_intersect_aabb(&aabb, &(bvh->aabb)) == false)
                continue;

            if (bvh->left >= 0)
            {
                stack[stack_index] = bvh->left;
                ++stack_index;
            }

            if (bvh->right >= 0)
            {
                stack[stack_index] = bvh->right;
                ++stack_index;
            }
        }
    }
}

void minimum_squared_distance(ObjData::Shape* shape, Vector3 query_point, float* out_signed_distance, Vector3* out_closest_point, int* out_face_index)
{
    int* stack = (int*)ALLOCA(sizeof(int) * shape->bvh_max_depth);
    assert(stack != NULL);

    int stack_index = 0;
    BVH* bvhptr = shape->bvhs.data();
    BVH* bvh = NULL;
    BVH* temp_bvh;

    stack[stack_index] = (int)shape->bvhs.size() - 1;
    ++stack_index;

    int tri_indices[3];
    Vector3 tri_verts[3];
    Vector3 closest_point;
    Vector3 temp_point;
    float closest_dist = FLT_MAX;
    float temp_dist;
    int closest_out_face_index;

    bool is_add_left;
    bool is_add_right;
    float sq_left_ex_dist;
    float sq_right_ex_dist;

    while (true)
    {
        if (stack_index <= 0)
            break;

        int bvh_index = stack[stack_index - 1];
        --stack_index;

        bvh = &(bvhptr[bvh_index]);
        if (is_bvh_leaf(bvh) == true)
        {
            int fi = bvh->face_index * 3;
            tri_indices[0] = shape->indices[fi] * 3;
            tri_indices[1] = shape->indices[fi + 1] * 3;
            tri_indices[2] = shape->indices[fi + 2] * 3;

            tri_verts[0] = vector3_setp(&(shape->positions[tri_indices[0]]));
            tri_verts[1] = vector3_setp(&(shape->positions[tri_indices[1]]));
            tri_verts[2] = vector3_setp(&(shape->positions[tri_indices[2]]));

            temp_point = triangle_closest_point(query_point, tri_verts[0], tri_verts[1], tri_verts[2]);
            temp_dist = vector3_distance_sq(temp_point, query_point);

            if (temp_dist < closest_dist)
            {
                closest_dist = temp_dist;
                closest_point = temp_point;
                closest_out_face_index = bvh->face_index;
            }
        }
        else
        {
            is_add_left = false;
            is_add_right = false;
            sq_left_ex_dist = FLT_MAX - 1.f;
            sq_right_ex_dist = FLT_MAX - 1.f;

            if (bvh->left >= 0)
            {
                temp_bvh = &(bvhptr[bvh->left]);

                sq_left_ex_dist = aabb_distance_exterior_sq_point(&(temp_bvh->aabb), query_point);

                if (aabb_contain_point(&(temp_bvh->aabb), query_point))
                {
                    stack[stack_index] = bvh->left;
                    ++stack_index;

                    is_add_left = true;
                }
            }

            if (bvh->right >= 0)
            {
                temp_bvh = &(bvhptr[bvh->right]);

                sq_right_ex_dist = aabb_distance_exterior_sq_point(&(temp_bvh->aabb), query_point);

                if (aabb_contain_point(&(temp_bvh->aabb), query_point))
                {
                    stack[stack_index] = bvh->right;
                    ++stack_index;

                    is_add_right = true;
                }
            }

            if (bvh->left >= 0 && is_add_left == false && sq_left_ex_dist < closest_dist)
            {
                stack[stack_index] = bvh->left;
                ++stack_index;
            }

            if (bvh->right >= 0 && is_add_right == false && sq_right_ex_dist < closest_dist)
            {
                stack[stack_index] = bvh->right;
                ++stack_index;
            }
        }
    }

    *out_signed_distance = closest_dist;
    *out_closest_point = closest_point;
    *out_face_index = closest_out_face_index;
}