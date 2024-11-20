#include "render_primitive.h"

#include <glad/glad.h>
#include "gl.h"

#include <glm/gtc/type_ptr.hpp>

void render_primitive_init(RenderPrimitive* rp)
{
	// point rendering init...
	const char* point_vs = \
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aColor;\n"
		"layout(location = 2) in float aSize;\n"
		"uniform mat4 proj_view;\n"
		"out vec4 pointColor;\n"
		"void main(void)\n"
		"{\n"
		"	gl_Position = proj_view * vec4(aPos, 1.0f);\n"
		"   gl_PointSize = aSize;\n"
		"	pointColor = vec4(aColor, 1.0);\n"
		"}\n";

	const char* point_fs = \
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec4 pointColor;\n"
		"void main(void)\n"
		"{\n"
		"	FragColor = pointColor;\n"
		"}\n";

    unsigned so[2];

	// vertex Shader
	so[0] = glCreateShader(GL_VERTEX_SHADER);
	gl_validate_shader(so[0], point_vs);
	
	// fragment shader
    so[1] = glCreateShader(GL_FRAGMENT_SHADER);
	gl_validate_shader(so[1], point_fs);

	// shader Program
	rp->point_program = glCreateProgram();
	gl_validate_program(rp->point_program, so, 2);

	rp->point_proj_view_loc = glGetUniformLocation(rp->point_program, "proj_view");

	// delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(so[0]);
	glDeleteShader(so[1]);

	glGenVertexArrays(1, &(rp->point_vao));
	glGenBuffers(3, rp->point_vbos);

	glBindVertexArray(rp->point_vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, rp->point_vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Color Buffer
	glBindBuffer(GL_ARRAY_BUFFER, rp->point_vbos[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Size Buffer
	glBindBuffer(GL_ARRAY_BUFFER, rp->point_vbos[2]);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	rp->point_vertices.clear();
	rp->point_colors.clear();
	rp->point_sizes.clear();

	// line rendering init...
	const char* line_vs = 
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aColor;\n"
		"uniform mat4 proj_view;\n"
		"out vec4 lineColor;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = proj_view * vec4(aPos, 1.0);\n"
		"	lineColor = vec4(aColor, 1.0);\n"
		"}";

	const char* line_fs = 
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec4 lineColor;\n"
		"void main()\n"
		"{\n"
		"	FragColor = lineColor;\n"
		"}";
    so[0] = glCreateShader(GL_VERTEX_SHADER);
	gl_validate_shader(so[0], line_vs);

    so[1] = glCreateShader(GL_FRAGMENT_SHADER);
	gl_validate_shader(so[1], line_fs);

	rp->line_program = glCreateProgram();
	gl_validate_program(rp->line_program, so, 2);

	rp->line_proj_view_loc = glGetUniformLocation(rp->line_program, "proj_view");

	glDeleteShader(so[0]);
	glDeleteShader(so[1]);

	glGenVertexArrays(1, &(rp->line_vao));
	glGenBuffers(2, rp->line_vbos);

	glBindVertexArray(rp->line_vao);
	glEnableVertexAttribArray(0); // vertex
	glEnableVertexAttribArray(1); // color

	// Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, rp->line_vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Color Buffer
	glBindBuffer(GL_ARRAY_BUFFER, rp->line_vbos[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	rp->line_vertices.clear();
	rp->line_colors.clear();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// primitive wireframe cube instance rendering
    /*
    const char* instance_vs =
        "#version 330 core\n"
        "layout(location = 0) in vec3 apos;\n"
        "layout(location = 1) in vec4 instance_color;\n"
        "layout(location = 2) in mat4 instance_model;\n"
        "uniform mat4 proj;\n"
        "uniform mat4 view;\n"
        "out vec4 v_color;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = proj * view * instance_model * vec4(apos, 1.0);\n"
        "   v_color = instance_color;\n"
        "}\n";

    const char* instance_fs =
        "#veresion 330 core\n"
        "in vec4 v_color;\n"
        "out vec4 frag_color;\n"
        "void main()"
        "{\n"
        "   frag_color = v_color;\n"
        "}\n";

    vertex = glCreateShader(GL_VERTEX_SHADER);
    gl_validate_shader(vertex, instance_vs);

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    gl_validate_shader(fragment, instance_fs);

    rp->instance_program = glCreateProgram();
    gl_validate_program(rp->instance_program, vertex, fragment);

    rp->instance_proj_loc = glGetUniformLocation(rp->instance_program, "proj");
    rp->instance_view_loc = glGetUniformLocation(rp->instance_program, "view");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    glGenVertexArrays(1, &(rp->instance_vao));
    glGenBuffers(3, rp->instance_vbos);

    glBindVertexArray(rp->instance_vao);
    glEnableVertexAttribArray(0); // vertex

    // Vertex Buffer
    glBindBuffer(GL_ARRAY_BUFFER, rp->instance_vbos[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Color Buffer
    glBindBuffer(GL_ARRAY_BUFFER, rp->instance_vbos[1]);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(1, 1);

    // instancing model matrix buffer
    glBindBuffer(GL_ARRAY_BUFFER, rp->instance_vbos[2]);
    for (int i = 2; i < 2 + 4; ++i)
    {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)((i - 2) * sizeof(float) * 4));
        glVertexAttribDivisor(i, 1);
    }

    rp->instance_models.clear();
    rp->instance_colors.clear();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    */
}

void render_primitive_terminate(RenderPrimitive* rp)
{
	glDeleteProgram(rp->line_program);
	glDeleteProgram(rp->point_program);
	
	rp->point_vertices.clear();
	rp->point_colors.clear();
	rp->point_sizes.clear();
	rp->line_vertices.clear();
	rp->line_colors.clear();
}

void render_primitive_insert_point(RenderPrimitive* rp, const Vector3& p, const Vector3& color, float size)
{
	rp->point_vertices.push_back(p);
	rp->point_colors.push_back(color);
	rp->point_sizes.push_back(size);
}

void render_primitive_insert_line(RenderPrimitive* rp, const Vector3& from, const Vector3& to, const Vector3& color)
{
	rp->line_vertices.push_back(from);
	rp->line_vertices.push_back(to);

    rp->line_colors.push_back(color);
	rp->line_colors.push_back(color);
}

void render_primitive_insert_line_colors(RenderPrimitive* rp, const Vector3& from, const Vector3& to, const Vector3& color1, const Vector3& color2)
{
    rp->line_vertices.push_back(from);
    rp->line_vertices.push_back(to);

    rp->line_colors.push_back(color1);
    rp->line_colors.push_back(color2);
}

void render_primitive_insert_wire_cube_lines(RenderPrimitive* rp, const Vector3& min_pos, const Vector3& max_pos, const Vector3& color)
{
	Vector3 extent = vector3_sub(max_pos, min_pos);

	static unsigned indices[] =
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0,
		0, 4,
		1, 5,
		2, 6,
		3, 7,
		4, 5,
		5, 6,
		6, 7,
		7, 4
	};

	Vector3 vertices[] =
	{
		min_pos,
        vector3_add(min_pos, vector3_set3(0.f, 0.f, extent.v[2])),
        vector3_add(min_pos, vector3_set3(extent.v[0], 0.f, extent.v[2])),
        vector3_add(min_pos, vector3_set3(extent.v[0], 0.f, 0.f)),
        vector3_sub(max_pos, vector3_set3(extent.v[0], 0.f, extent.v[2])),
        vector3_sub(max_pos, vector3_set3(extent.v[0], 0.f, 0.f)),		
		max_pos,
        vector3_sub(max_pos, vector3_set3(0.f, 0.f, extent.v[2]))
	};

	for (int i = 0; i < sizeof(indices) / sizeof(indices[0]); i += 2)
	{
		unsigned idx_from = indices[i];
		unsigned idx_to = indices[i + 1];

		render_primitive_insert_line(rp, vertices[idx_from], vertices[idx_to], color);
	}
}

void render_primitive_insert_triangle_lines(RenderPrimitive* rp, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& color)
{
    render_primitive_insert_line(rp, a, b, color);
    render_primitive_insert_line(rp, b, c, color);
    render_primitive_insert_line(rp, c, a, color);
}

void render_primitive_insert_triangle_normal_lines(RenderPrimitive* rp, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& norm, float norm_scale, const Vector3& color)
{
    Vector3 scale_norm = vector3_mul_scalar(norm, norm_scale);
    render_primitive_insert_line(rp, a, vector3_add(a, scale_norm), color);
    render_primitive_insert_line(rp, b, vector3_add(b, scale_norm), color);
    render_primitive_insert_line(rp, c, vector3_add(c, scale_norm), color);
}

void render_primitive_clear_primitives(RenderPrimitive* rp)
{
	rp->point_vertices.clear();
	rp->point_colors.clear();
	rp->point_sizes.clear();
	rp->line_vertices.clear();
	rp->line_colors.clear();
}

void render_primitive_clear_points(RenderPrimitive* rp)
{
	rp->point_vertices.clear();
	rp->point_colors.clear();
	rp->point_sizes.clear();
}

void render_primitive_clear_lines(RenderPrimitive* rp)
{
	rp->line_vertices.clear();
	rp->line_colors.clear();
}

void render_primitive_render(RenderPrimitive* rp, const glm::mat4& proj, const glm::mat4& view, float line_width)
{
	glm::mat4 proj_view = proj * view;

	// render points
	if (rp->point_vertices.size() > 0)
	{
		size_t count = rp->point_vertices.size();
		size_t v_size = count * sizeof(glm::vec3);

		// Vertex Buffer
		glBindBuffer(GL_ARRAY_BUFFER, rp->point_vbos[0]);
		glBufferData(GL_ARRAY_BUFFER, v_size, rp->point_vertices.data(), GL_DYNAMIC_DRAW);

		// Color Buffer
		glBindBuffer(GL_ARRAY_BUFFER, rp->point_vbos[1]);
		glBufferData(GL_ARRAY_BUFFER, v_size, rp->point_colors.data(), GL_DYNAMIC_DRAW);

		v_size = count * sizeof(float);

		// Size Buffer
		glBindBuffer(GL_ARRAY_BUFFER, rp->point_vbos[2]);
		glBufferData(GL_ARRAY_BUFFER, v_size, rp->point_sizes.data(), GL_DYNAMIC_DRAW);

		glUseProgram(rp->point_program);
		{
			glUniformMatrix4fv(rp->point_proj_view_loc, 1, GL_FALSE, glm::value_ptr(proj_view));
			glBindVertexArray(rp->point_vao);

			glEnable(GL_PROGRAM_POINT_SIZE);
			glDrawArrays(GL_POINTS, 0, (GLsizei)count);
		}

		// Setting Default again
		glDisable(GL_PROGRAM_POINT_SIZE);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (rp->line_vertices.size() > 0)
	{
		size_t count = rp->line_vertices.size();
		size_t v_size = count * sizeof(glm::vec3);

		glBindBuffer(GL_ARRAY_BUFFER, rp->line_vbos[0]);
		glBufferData(GL_ARRAY_BUFFER, v_size, rp->line_vertices.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, rp->line_vbos[1]);
		glBufferData(GL_ARRAY_BUFFER, v_size, rp->line_colors.data(), GL_DYNAMIC_DRAW);

		glUseProgram(rp->line_program);
		{
			glUniformMatrix4fv(rp->line_proj_view_loc, 1, GL_FALSE, glm::value_ptr(proj_view));
			glBindVertexArray(rp->line_vao);

			glLineWidth(line_width);
			glDrawArrays(GL_LINES, 0, (GLsizei)count);
		}

		// Setting Default again
		glLineWidth(1.0);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}