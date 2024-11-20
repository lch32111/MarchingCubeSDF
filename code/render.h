#ifndef __RENDER_H__
#define __RENDER_H__

#include <glm/glm.hpp>
#include "common.h"
#include "gl.h"
#include "camera.h"
#include "vector.h"

struct Camera;
struct RenderPrimitive;
struct SDFObjData;

struct GPUBuffer
{
	unsigned vao;
	unsigned vbos[8];
	unsigned vbo_count;
	unsigned ibo;
};

struct SDFGPUBuffer
{
    unsigned vao;
    unsigned vbo;
    unsigned tex;
};

struct Renderer
{
    std::vector<SDFObjData*> sdf_objs;
    std::vector<std::vector<GPUBuffer>> obj_buffers;
    std::vector<std::vector<SDFGPUBuffer>> sdf_buffers;
    std::vector<std::vector<std::vector<Vector3>>> sdf_debug_grid_points;

    std::vector<Vector3> obj_transform_pos;
    std::vector<Vector3> sdf_transform_pos;
    

    Camera cam;

	RenderPrimitive* render_primitive;

	ShaderProgram object_shader;
    ShaderProgram marchingcubes_shader;
    unsigned mcs_edge_table;
    unsigned mcs_tri_table;

	glm::vec3 sun_dir;
	glm::vec3 sun_ambient;
	glm::vec3 sun_diffuse;
	glm::vec3 sun_specular;
	glm::vec3 mat_ambient;
	glm::vec3 mat_diffuse;
	glm::vec3 mat_specular;
	float mat_shininess;
};

void renderer_init(Renderer* r, const std::vector<SDFObjData*>& sdf_objs);
void renderer_terminate(Renderer* r);
void renderer_update(Renderer* r);
void renderer_render(Renderer* r);

#endif