#ifndef __GL_H__
#define __GL_H__

void gl_check_error(const char* file, int line);

void gl_validate_shader(unsigned so, const char* shader_source);

void gl_validate_program(unsigned pso, unsigned* shaders, unsigned shader_count);

struct ShaderProgram
{
	unsigned pso;
};

ShaderProgram gl_create_program_from_shaders(const char* vs, const char* fs);
ShaderProgram gl_create_program_from_shader_with_geometry(const char* vs, const char* gs, const char* fs);

void gl_destroy_program(ShaderProgram sp);

#endif
