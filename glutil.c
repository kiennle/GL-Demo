#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include "glutil.h"

void gl_map_write_buffer(GLuint buffer, GLintptr offset,
			 GLsizeiptr size, const void *data)
{
	void *mapped = glMapNamedBufferRange(buffer, offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);

	assert(mapped);
	memcpy(mapped, data, size);
	glUnmapNamedBuffer(buffer);
}

GLuint gl_create_buffer(GLsizei size, const void *data, GLbitfield flags)
{
	GLuint buffer;

	glCreateBuffers(1, &buffer);
	glNamedBufferStorage(buffer, size, data, flags);
	return buffer;
}

static char *load_text(const char *path, int *len)
{
	FILE *fp;
	struct stat st;
	char *text;

	fp = fopen(path, "rb");
	if (!fp) {
		fprintf(stderr, "fopen %s failed: %d %s\n", path, errno, strerror(errno));
		return NULL;
	}

	fstat(fileno(fp), &st);
	*len = st.st_size;
	text = (char *)malloc(*len + 1);
	if (text) {
		fread(text, 1, st.st_size, fp);
		text[st.st_size] = '\0';
	}
	fclose(fp);
	return text;
}

GLuint gl_create_shader(const char *src, GLenum type)
{
	GLuint shader;
	GLint status, infolen;

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);
	if (infolen != 0) {
		GLchar *info = (GLchar *)malloc(infolen);

		glGetShaderInfoLog(shader, infolen, NULL, info);
		printf("%s\n", info);
		free(info);
	}
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
		fprintf(stderr, "Compile shader failed.\n");
	return shader;
}

GLuint gl_load_shader(const char *path, GLenum type)
{
	int tmp;
	char *src = load_text(path, &tmp);
	GLuint shader = gl_create_shader(src, type);

	free(src);
	return shader;
}

GLuint gl_create_program(GLuint vs, GLuint fs)
{
	GLint status, infolen;
	GLuint program = glCreateProgram();

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infolen);
	if (infolen != 0) {
		GLchar *info = (GLchar *)malloc(infolen);

		glGetProgramInfoLog(program, infolen, NULL, info);
		printf("%s\n", info);
		free(info);
	}
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
		fprintf(stderr, "Link shader program failed.\n");
	glDetachShader(program, vs);
	glDetachShader(program, fs);
	return program;
}

GLuint gl_load_program(const char *vs_path, const char *fs_path)
{
	GLuint vs = gl_load_shader(vs_path, GL_VERTEX_SHADER);
	GLuint fs = gl_load_shader(fs_path, GL_FRAGMENT_SHADER);
	GLuint program = gl_create_program(vs, fs);

	glDeleteShader(vs);
	glDeleteShader(fs);
	return program;
}

static int calc_miplevels(int width, int height) {
	int levels = 1;
	while ((width | height) >> levels)
		levels += 1;
	return levels;
}

GLuint gl_create_texture_2d(GLenum fmt, int w, int h, bool hasmips, GLenum filter, GLenum wrap)
{
	GLuint id;
	int miplevels = hasmips ? calc_miplevels(w, h) : 1;

	glCreateTextures(GL_TEXTURE_2D, 1, &id);
	glTextureStorage2D(id, miplevels, fmt, w, h);
	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, filter);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, filter);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap);
	//glTextureSubImage2D(id, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	if (miplevels > 1)
		glGenerateTextureMipmap(id);
	return id;
}

GLuint gl_create_fbo(GLuint depth_stencil, int nr_colorbufs, ...)
{
	GLuint fbo;
	int i;
	va_list va;

	glCreateFramebuffers(1, &fbo);
	va_start(va, nr_colorbufs);
	for (i = 0; i < nr_colorbufs; i++) {
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0 + i, va_arg(va, GLuint), 0);
		glNamedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0 + i);
	}
	va_end(va);
	if (glIsTexture(depth_stencil) == GL_TRUE)
		glNamedFramebufferTexture(fbo, GL_DEPTH_STENCIL_ATTACHMENT, depth_stencil, 0);
	if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		abort();
	return fbo;
}
