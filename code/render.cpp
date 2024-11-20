#include "render.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "obj.h"
#include "sdf_obj.h"
#include "render_primitive.h"
#include "marching_cubes.h"
#include "window.h"

static inline void delete_gpu_buffer(const GPUBuffer& gpub)
{
	glDeleteBuffers(gpub.vbo_count, gpub.vbos);
	glDeleteBuffers(1, &(gpub.ibo));
	glDeleteVertexArrays(1, &(gpub.vao));
}

static inline void delete_sdf_gpu_buffer(const SDFGPUBuffer& gpub)
{
    glDeleteBuffers(1, &(gpub.vbo));
    glDeleteVertexArrays(1, &(gpub.vao));

    if (gpub.tex != 0)
    {
        glDeleteTextures(1, &(gpub.tex));
    }
}

void renderer_init(Renderer* r, const std::vector<SDFObjData*>& sdf_objs)
{
    r->sdf_objs = sdf_objs;
    r->obj_buffers.resize(sdf_objs.size());
    r->sdf_buffers.resize(sdf_objs.size());
    r->sdf_debug_grid_points.resize(sdf_objs.size());

    r->obj_transform_pos.resize(sdf_objs.size());
    r->sdf_transform_pos.resize(sdf_objs.size());

    std::vector<Vector3> temp_voxel_center_array;
    for (size_t si = 0; si < sdf_objs.size(); ++si)
    {
        SDFObjData* sod = sdf_objs[si];
        ObjData* od = sdf_objs[si]->data;
        size_t shape_count = od->shapes.size();

        std::vector<GPUBuffer>& obj_buffers = r->obj_buffers[si];
        std::vector<SDFGPUBuffer>& sdf_buffers = r->sdf_buffers[si];
        std::vector<std::vector<Vector3>>& sdf_debug_grid_points = r->sdf_debug_grid_points[si];

        obj_buffers.resize(shape_count);
        sdf_buffers.resize(shape_count);
        sdf_debug_grid_points.resize(shape_count);

        // obj buffer first
        for (size_t ssi = 0; ssi < shape_count; ++ssi)
        {
            ObjData::Shape& shape = od->shapes[ssi];
            GPUBuffer& gpub = obj_buffers[ssi];

            glGenVertexArrays(1, &gpub.vao);

            glBindVertexArray(gpub.vao);

            // pos, normal
            gpub.vbo_count = 2;
            for (size_t vbo_index = 0; vbo_index < gpub.vbo_count; ++vbo_index)
            {
                glEnableVertexAttribArray((GLuint)vbo_index);
            }

            glGenBuffers(3, gpub.vbos);
            gpub.ibo = gpub.vbos[2];

            glBindBuffer(GL_ARRAY_BUFFER, gpub.vbos[0]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * shape.positions.size(), shape.positions.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, gpub.vbos[1]);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * shape.normals.size(), shape.normals.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpub.ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * shape.indices.size(), shape.indices.data(), GL_STATIC_DRAW);

            glBindVertexArray(0);
        }

        // sdf buffer next
        for (size_t ssi = 0; ssi < shape_count; ++ssi)
        {
            ObjData::Shape& shape = od->shapes[ssi];
            SDFGPUBuffer& gpub = sdf_buffers[ssi];
            Grid& shape_grid = sod->grids[si];
            std::vector<Vector3>& sdf_debug_grid_point = sdf_debug_grid_points[ssi];
            sdf_debug_grid_point.reserve(shape_grid.nz * shape_grid.ny * shape_grid.nx);

            Vector3 p;
            temp_voxel_center_array.clear();
            for (int k = 0; k < shape_grid.nz; ++k)
            {
                p.v[2] = shape_grid.min_pos[2] + sod->grid_delta * (k);
                for (int j = 0; j < shape_grid.ny; ++j)
                {
                    p.v[1] = shape_grid.min_pos[1] + sod->grid_delta * (j);

                    for (int i = 0; i < shape_grid.nx; ++i)
                    {
                        p.v[0] = shape_grid.min_pos[0] + sod->grid_delta * (i);

                        temp_voxel_center_array.push_back(vector3_add(p, vector3_mul_scalar(vector3_set1(sod->grid_delta), 0.5f)));
                        sdf_debug_grid_point.push_back(p);
                    }
                }
            }

            glGenVertexArrays(1, &(gpub.vao));
            glBindVertexArray(gpub.vao);

            glGenBuffers(1, &(gpub.vbo));
            glBindBuffer(GL_ARRAY_BUFFER, gpub.vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * temp_voxel_center_array.size(), temp_voxel_center_array.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

            glGenTextures(1, &(gpub.tex));
            glBindTexture(GL_TEXTURE_3D, gpub.tex);
            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, shape_grid.nx, shape_grid.ny, shape_grid.nz, 0, GL_RED, GL_FLOAT, shape_grid.sdfs.data());

            glBindVertexArray(0);
        }
    }

    camera_reset(&r->cam, glm::vec3(0.f), g_window_state.window_width, g_window_state.window_height);

	r->render_primitive = new RenderPrimitive();
	render_primitive_init(r->render_primitive);

	r->object_shader = gl_create_program_from_shaders("resource/object.vs", "resource/object.fs");

    r->marchingcubes_shader = gl_create_program_from_shader_with_geometry("resource/marching_cubes.vs", "resource/marching_cubes.gs", "resource/marching_cubes.fs");
    r->mcs_edge_table = create_gl_1d_edge_table();
    r->mcs_tri_table = create_gl_1d_tri_table();

	r->sun_dir = glm::normalize(glm::vec3(0.f, 1.f, 0.6f)); 
	r->sun_ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	r->sun_diffuse = glm::vec3(0.883f, 0.883f, 0.883f);
	r->sun_specular = glm::vec3(0.883f, 0.883f, 0.883f);
	r->mat_ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	r->mat_diffuse = glm::vec3(0.621f, 0.621f, 0.621f);
	r->mat_specular = glm::vec3(0.4f, 0.2f, 0.1f);
	r->mat_shininess = 64.f;
}

void renderer_terminate(Renderer* r)
{
    glDeleteTextures(1, &(r->mcs_tri_table));
    glDeleteTextures(1, &(r->mcs_edge_table));

    gl_destroy_program(r->marchingcubes_shader);
	gl_destroy_program(r->object_shader);

	render_primitive_terminate(r->render_primitive);
	delete r->render_primitive;
}

void renderer_update(Renderer* r)
{
    WindowState& ws = g_window_state;
    camera_update(&r->cam, ws.window_width, ws.window_height);
}

static 
void obj_render(Renderer* r, size_t obj_index)
{
    SDFObjData* sdf_obj = r->sdf_objs[obj_index];
    ObjData* od = sdf_obj->data;
    const std::vector<GPUBuffer>& obj_buffers = r->obj_buffers[obj_index];
	const int shape_count = (int)od->shapes.size();
    Vector3 obj_pos = r->obj_transform_pos[obj_index];

    GLuint pso = r->object_shader.pso;
    glUseProgram(pso);

    float model[] = { 1.f, 0.f, 0.f, 0.f,
                        0.f, 1.f, 0.f, 0.f,
                        0.f, 0.f, 1.f, 0.f,
                        obj_pos.v[0], obj_pos.v[1], obj_pos.v[2], 1.f};

    glUniformMatrix4fv(glGetUniformLocation(pso, "model"), 1, GL_FALSE, model);
    
    glUniformMatrix4fv(glGetUniformLocation(pso, "view"), 1, GL_FALSE, glm::value_ptr(r->cam.view));
    glUniformMatrix4fv(glGetUniformLocation(pso, "projection"), 1, GL_FALSE, glm::value_ptr(r->cam.projection));
    glUniform3fv(glGetUniformLocation(pso, "cam_pos"), 1, glm::value_ptr(r->cam.position));
    glUniform3fv(glGetUniformLocation(pso, "sun_dir"), 1, glm::value_ptr(r->sun_dir));
    glUniform3fv(glGetUniformLocation(pso, "sun_ambient"), 1, glm::value_ptr(r->sun_ambient));
    glUniform3fv(glGetUniformLocation(pso, "sun_diffuse"), 1, glm::value_ptr(r->sun_diffuse));
    glUniform3fv(glGetUniformLocation(pso, "sun_specular"), 1, glm::value_ptr(r->sun_specular));
    glUniform3fv(glGetUniformLocation(pso, "mat_ambient"), 1, glm::value_ptr(r->mat_ambient));
    glUniform3fv(glGetUniformLocation(pso, "mat_diffuse"), 1, glm::value_ptr(r->mat_diffuse));
    glUniform3fv(glGetUniformLocation(pso, "mat_specular"), 1, glm::value_ptr(r->mat_specular));
    glUniform1f(glGetUniformLocation(pso, "mat_shininess"), r->mat_shininess);

	for (int si = 0; si < shape_count; ++si)
	{
		const GPUBuffer& gpub = obj_buffers[si];
		const ObjData::Shape& shape = od->shapes[si];

		glBindVertexArray(gpub.vao);
		glDrawElements(GL_TRIANGLES, (GLsizei)shape.indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}
}

void sdf_obj_render(Renderer* r, size_t sdf_obj_index)
{
    SDFObjData* sod = r->sdf_objs[sdf_obj_index];
    std::vector<SDFGPUBuffer>& sdf_buffers = r->sdf_buffers[sdf_obj_index];
    std::vector<std::vector<Vector3>> sdf_grid_points = r->sdf_debug_grid_points[sdf_obj_index];
    Vector3 sdf_pos = r->sdf_transform_pos[sdf_obj_index];

    // if(sod->render_mesh_by_marching_cubes == false)
	    obj_render(r, sdf_obj_index);

	ObjData* od = sod->data;
    size_t shape_count = (int)od->shapes.size();

    float model[] = { 1.f, 0.f, 0.f, 0.f,
                      0.f, 1.f, 0.f, 0.f,
                      0.f, 0.f, 1.f, 0.f,
                      sdf_pos.v[0], sdf_pos.v[1], sdf_pos.v[2], 1.f};
    GLuint pso = r->marchingcubes_shader.pso;
    glUseProgram(pso);
    glUniformMatrix4fv(glGetUniformLocation(pso, "model"), 1, GL_FALSE, model);
    glUniformMatrix4fv(glGetUniformLocation(pso, "view"), 1, GL_FALSE, glm::value_ptr(r->cam.view));
    glUniformMatrix4fv(glGetUniformLocation(pso, "projection"), 1, GL_FALSE, glm::value_ptr(r->cam.projection));
    glUniform3fv(glGetUniformLocation(pso, "cam_pos"), 1, glm::value_ptr(r->cam.position));
    glUniform3fv(glGetUniformLocation(pso, "sun_dir"), 1, glm::value_ptr(r->sun_dir));
    glUniform3fv(glGetUniformLocation(pso, "sun_ambient"), 1, glm::value_ptr(r->sun_ambient));
    glUniform3fv(glGetUniformLocation(pso, "sun_diffuse"), 1, glm::value_ptr(r->sun_diffuse));
    glUniform3fv(glGetUniformLocation(pso, "sun_specular"), 1, glm::value_ptr(r->sun_specular));
    glUniform3fv(glGetUniformLocation(pso, "mat_ambient"), 1, glm::value_ptr(r->mat_ambient));
    glUniform3fv(glGetUniformLocation(pso, "mat_diffuse"), 1, glm::value_ptr(r->mat_diffuse));
    glUniform3fv(glGetUniformLocation(pso, "mat_specular"), 1, glm::value_ptr(r->mat_specular));
    glUniform1f(glGetUniformLocation(pso, "mat_shininess"), r->mat_shininess);

    for (size_t si = 0; si < shape_count; ++si)
    {
        ObjData::Shape& shape = od->shapes[si];
        Grid& shape_grid = sod->grids[si];
        SDFGPUBuffer& gpub = sdf_buffers[si];
        std::vector<Vector3>& grid_points = sdf_grid_points[si];


        if (sod->render_bounds)
        {
            Vector3 min_pos = { shape.min_positions[0], shape.min_positions[1], shape.min_positions[2] };
            min_pos = vector3_add(sdf_pos, min_pos);
            Vector3 max_pos = { shape.max_positions[0], shape.max_positions[1], shape.max_positions[2] };
            max_pos = vector3_add(sdf_pos, max_pos);
            render_primitive_insert_wire_cube_lines(r->render_primitive, min_pos, max_pos, vector3_set3(0.8f, 0.7f, 0.f));
        }

        if (sod->render_bvh)
        {
            for (const BVH& bvh : shape.bvhs)
            {
                Vector3 min_p = vector3_add(sdf_pos, bvh.aabb.min_p);
                Vector3 max_p = vector3_add(sdf_pos, bvh.aabb.max_p);
                render_primitive_insert_wire_cube_lines(r->render_primitive, min_p, max_p, vector3_set3(0.4f, 0.7f, 0.5f));
            }
        }

        if (sod->render_grid_points || sod->render_sdf_debug_info)
        {
            Vector3 grid_point_color = VECTOR3_COLOR_WHITE;
            for (size_t sdi = 0; sdi < shape_grid.sdf_debugs.size(); ++sdi)
            {
                float sd_value = shape_grid.sdfs[sdi];
                const SDFDebug& sd = shape_grid.sdf_debugs[sdi];
                Vector3 grid_point = vector3_add(sdf_pos, grid_points[sdi]);

                if (sod->render_grid_points)
                {
                    if (sd_value > 0.f)
                    {
                        grid_point_color = VECTOR3_COLOR_RED;
                    }
                    else if (sd_value == 0.f)
                    {
                        grid_point_color = VECTOR3_COLOR_GREEN;
                    }
                    else
                    {
                        grid_point_color = VECTOR3_COLOR_BLUE;
                    }

                    if (sd.is_set == false)
                    {
                        grid_point_color = VECTOR3_COLOR_LAVENDER;
                    }

                    render_primitive_insert_point(r->render_primitive, grid_point, grid_point_color, 1.5f);
                }

                if (sod->render_sdf_debug_info && sd.is_set)
                {
                    Vector3 tri0 = vector3_add(sdf_pos, sd.tri[0]);
                    Vector3 tri1 = vector3_add(sdf_pos, sd.tri[1]);
                    Vector3 tri2 = vector3_add(sdf_pos, sd.tri[2]);
                    Vector3 closest_tri_pos = vector3_add(sdf_pos, sd.closest_tri_pos);

                    render_primitive_insert_triangle_lines(r->render_primitive, tri0, tri1, tri2, VECTOR3_COLOR_SKY_BLUE);
                    if (sod->render_sdf_debug_triangle_normal)
                        render_primitive_insert_triangle_normal_lines(r->render_primitive, tri0, tri1, tri2, sd.closest_tri_normal, 0.05f, VECTOR3_COLOR_YELLOW);
                    render_primitive_insert_point(r->render_primitive, closest_tri_pos, VECTOR3_COLOR_MAGENTA, 3.f);
                    render_primitive_insert_line_colors(r->render_primitive, closest_tri_pos, grid_point, VECTOR3_COLOR_MAGENTA, grid_point_color);
                }
            }
        }


        if (sod->render_mesh_by_marching_cubes)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, gpub.tex);
            glUniform1i(glGetUniformLocation(pso, "tex_sdf"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_1D, r->mcs_edge_table);
            glUniform1i(glGetUniformLocation(pso, "tex_edge_table"), 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_1D, r->mcs_tri_table);
            glUniform1i(glGetUniformLocation(pso, "tex_tri_table"), 2);

            glUniform3fv(glGetUniformLocation(pso, "sdf_origin"), 1, shape_grid.min_pos);

            float sdf_dimension[4] = { shape_grid.dimensions[0], shape_grid.dimensions[1], shape_grid.dimensions[2], sod->grid_delta };
            glUniform4fv(glGetUniformLocation(pso, "sdf_dimension"), 1, sdf_dimension);
            glUniform1f(glGetUniformLocation(pso, "iso_value"), sod->iso_value);

            glBindVertexArray(gpub.vao);
            glDrawArrays(GL_POINTS, 0, (GLsizei)shape_grid.sdfs.size());
        }
    }

    glBindVertexArray(0);
}

void renderer_render(Renderer* r)
{
    WindowState& ws = g_window_state;
	glViewport(0, 0, ws.window_width, ws.window_height); 
	glEnable(GL_DEPTH_TEST); 

    for (size_t si = 0; si < r->sdf_objs.size(); ++si)
    {
        sdf_obj_render(r, si);
    }

	render_primitive_render(r->render_primitive, r->cam.projection, r->cam.view, 1.f);
	render_primitive_clear_primitives(r->render_primitive);
}