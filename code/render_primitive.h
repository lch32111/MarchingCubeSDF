#ifndef __RENDER_PRIMITIVE_H__
#define __RENDER_PRIMITIVE_H__

#include <vector>
#include "vector.h"
#include <glm/glm.hpp>

struct RenderPrimitive
{
	unsigned point_vao;
	unsigned point_vbos[3];
	unsigned point_program;
	unsigned point_proj_view_loc;

	std::vector<Vector3> point_vertices;
	std::vector<Vector3> point_colors;
	std::vector<float> point_sizes;

	unsigned line_vao;
	unsigned line_vbos[2];
	unsigned line_program;
	unsigned line_proj_view_loc;

	std::vector<Vector3> line_vertices;
	std::vector<Vector3> line_colors;

    /*
    unsigned instance_vao;
    unsigned instance_vbos[3];
    unsigned instance_program;
    unsigned instance_proj_loc;
    unsigned instance_view_loc;
    std::vector<Eigen::Matrix4f> instance_models;
    std::vector<Eigen::Vector4f> instance_colors;
    */
};

void render_primitive_init(RenderPrimitive* rp);
void render_primitive_terminate(RenderPrimitive* rp);

void render_primitive_insert_point(RenderPrimitive* rp, const Vector3& p, const Vector3& color, float size);
void render_primitive_insert_line(RenderPrimitive* rp, const Vector3& from, const Vector3& to, const Vector3& color);
void render_primitive_insert_line_colors(RenderPrimitive* rp, const Vector3& from, const Vector3& to, const Vector3& color1, const Vector3& color2);
void render_primitive_insert_wire_cube_lines(RenderPrimitive* rp, const Vector3& min_pos, const Vector3& max_pos, const Vector3& color);
void render_primitive_insert_triangle_lines(RenderPrimitive* rp, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& color);
void render_primitive_insert_triangle_normal_lines(RenderPrimitive* rp, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& norm, float norm_scale, const Vector3& color);

void render_primitive_clear_primitives(RenderPrimitive* rp);
void render_primitive_clear_points(RenderPrimitive* rp);
void render_primitive_clear_lines(RenderPrimitive* rp);

void render_primitive_render(RenderPrimitive* rp, const glm::mat4& proj, const glm::mat4& view, float line_width);

#endif