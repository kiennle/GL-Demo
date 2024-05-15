#pragma once
#include <glad/glad.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void gl_map_write_buffer(GLuint buffer, GLintptr offset,
			 GLsizeiptr size, const void *data);
GLuint gl_create_buffer(GLsizei size, const void *data, GLbitfield flags);
GLuint gl_create_shader(const char *src, GLenum type);
GLuint gl_load_shader(const char *path, GLenum type);
GLuint gl_create_program(GLuint vs, GLuint fs);
GLuint gl_load_program(const char *vs_path, const char *fs_path);
GLuint gl_create_texture_2d(GLenum fmt, int w, int h, bool genmips, GLenum filter, GLenum wrap);
GLuint gl_create_fbo(GLuint depth_stencil, int nr_colorbufs, ...);

#ifdef __cplusplus
}
#endif
