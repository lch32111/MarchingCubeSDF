#include "gl.h"

#include <stdio.h>
#include <glad/glad.h>
#include <assert.h>

#include "common.h"

void gl_check_error(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		const char* error = "ERROR_LOG_NONE";
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}

		printf("Error %s | %s ( %d )", error, file, line);
	}
}

void gl_validate_shader(unsigned so, const char* shader_source)
{
	glShaderSource(so, 1, &shader_source, NULL);
	glCompileShader(so);
	{
		GLint success;
		glGetShaderiv(so, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char info_logs[512];
			glGetShaderInfoLog(so, 512, NULL, info_logs);
			printf("Fail to Compile source : %s\n", info_logs);
			assert(false);
		}
	}
}

void gl_validate_program(unsigned pso, unsigned* shaders, unsigned shader_count)
{
    for (unsigned i = 0; i < shader_count; ++i)
    {
        glAttachShader(pso, shaders[i]);
    }

	glLinkProgram(pso);
	{
		GLint success;
		glGetProgramiv(pso, GL_LINK_STATUS, &success);
		if (!success)
		{
			char info_logs[512];
			glGetProgramInfoLog(pso, 512, NULL, info_logs);
			printf("Fail to Compile source : %s\n", info_logs);
			assert(false);
		}
	}
}

ShaderProgram gl_create_program_from_shaders(const char* vs, const char* fs)
{
	ShaderProgram sp;
	std::vector<char> buffer;
    unsigned so[2];

	file_open_fill_buffer(vs, buffer);

	so[0] = glCreateShader(GL_VERTEX_SHADER);
	gl_validate_shader(so[0], (const char*)buffer.data());

	file_open_fill_buffer(fs, buffer);
	so[1] = glCreateShader(GL_FRAGMENT_SHADER);
	gl_validate_shader(so[1], (const char*)buffer.data());

	GLuint pso = glCreateProgram();
	gl_validate_program(pso, so, 2);
	sp.pso = pso;

    glDeleteShader(so[1]);
    glDeleteShader(so[0]);

	return sp;
}

ShaderProgram gl_create_program_from_shader_with_geometry(const char* vs, const char* gs, const char* fs)
{
    ShaderProgram sp;
    std::vector<char> buffer;
    unsigned so[3];

    file_open_fill_buffer(vs, buffer);
    so[0] = glCreateShader(GL_VERTEX_SHADER);
    gl_validate_shader(so[0], (const char*)buffer.data());

    file_open_fill_buffer(gs, buffer);
    so[1] = glCreateShader(GL_GEOMETRY_SHADER);
    gl_validate_shader(so[1], (const char*)buffer.data());

    file_open_fill_buffer(fs, buffer);
    so[2] = glCreateShader(GL_FRAGMENT_SHADER);
    gl_validate_shader(so[2], (const char*)buffer.data());
    

    GLuint pso = glCreateProgram();
    gl_validate_program(pso, so, 3);
    sp.pso = pso;

    glDeleteShader(so[2]);
    glDeleteShader(so[1]);
    glDeleteShader(so[0]);

    return sp;
}

void gl_destroy_program(ShaderProgram sp)
{
	glDeleteProgram(sp.pso);
}
