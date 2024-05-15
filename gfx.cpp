#include "core.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "glutil.h"

enum {
	SLOT_POSITION = 0,
	SLOT_NORTAN,
	SLOT_UV,
	MAX_VBUF_SLOTS
};

enum {
	SLOT_ALBEDO_MAP,
	SLOT_NORMAL_MAP,
	SLOT_SPECULAR_MAP,
	SLOT_EMISSIVE_MAP,
};

struct draw_element {
	float4x4 matrix_world;
	struct mesh *mesh;
	int mesh_part;
	struct material *mat;
};

struct camera *main_cam;
struct listnode mesh_ent_list;
struct listnode light_list;

static struct fb_binding main_fb_binding;

static int scr_w, scr_h;

void gfx_init(float width, float height)
{
	scr_w = width;
	scr_h = height;
	main_cam = NULL;
	listnode_init(&light_list);
	listnode_init(&mesh_ent_list);

	main_fb_binding.fbo = 0;
	main_fb_binding.nr_color_bindings = 1;
	main_fb_binding.color_bindings[0].action = FBO_CLEAR;
	main_fb_binding.color_bindings[0].color = float4_from_xyzw(0.2f, 0.3f, 0.3f, 1.0f);
	main_fb_binding.enable_depth_stencil = true;
	main_fb_binding.depth_stencil_binding.depth_action = FBO_CLEAR;
	main_fb_binding.depth_stencil_binding.depth = 1.0f;
	main_fb_binding.depth_stencil_binding.stencil_action = FBO_DONTCARE;
	main_fb_binding.depth_stencil_binding.stencil = 0;

	//main_step.viewport.x = 0.0f;
	//main_step.viewport.y = 0.0f;
	//main_step.viewport.w = width;
	//main_step.viewport.h = height;
	//main_step.viewport.zn = -1.0f; /* NOTE: Set this to 0 for Direct3D */
	//main_step.viewport.zf = 1.0f;
	//main_step.fb_binding = main_fb_binding;
	//main_step.matrix_v = ;

	/*sunlight_shadow_map = gl_create_texture_2d(GL_DEPTH32F_STENCIL8, SHADOW_MAP_W, SHADOW_MAP_H, false, GL_NEAREST, GL_REPEAT);
	shadow_fb_binding.fbo = gl_create_fbo(sunlight_shadow_map, 0);
	shadow_fb_binding.nr_color_bindings = 0;
	shadow_fb_binding.enable_depth_stencil = true;
	shadow_fb_binding.depth_stencil_binding.depth_action = FBO_CLEAR;
	shadow_fb_binding.depth_stencil_binding.depth = 1.0f;
	shadow_fb_binding.depth_stencil_binding.stencil_action = FBO_PRESERVE;
	shadow_fb_binding.depth_stencil_binding.stencil = 0;

	depth_shdr = getorload_shader("shared/shdr_depth.json");*/
}

static inline GLenum to_gl_depth_func(enum depth_compare_func cmpfunc)
{
	switch (cmpfunc) {
	case DEPTH_COMPARE_NEVER: return GL_NEVER;
	case DEPTH_COMPARE_LESS: return GL_LESS;
	case DEPTH_COMPARE_EQUAL: return GL_EQUAL;
	case DEPTH_COMPARE_LESS_EQUAL: return GL_LEQUAL;
	case DEPTH_COMPARE_GREATER: return GL_GREATER;
	case DEPTH_COMPARE_NOT_EQUAL: return GL_NOTEQUAL;
	case DEPTH_COMPARE_GREATER_EQUAL: return GL_GEQUAL;
	case DEPTH_COMPARE_ALWAYS: return GL_ALWAYS;
	}
	abort();
}

static inline GLenum to_gl_blend_op(enum blend_op op)
{
	switch (op) {
	case BLEND_OP_ADD: return GL_FUNC_ADD;
	case BLEND_OP_SUBTRACT: return GL_FUNC_SUBTRACT;
	case BLEND_OP_REV_SUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
	case BLEND_OP_MIN: return GL_MIN;
	case BLEND_OP_MAX: return GL_MAX;
	}
	abort();
}

static inline GLenum to_gl_blend_factor(enum blend_factor factor)
{
	switch (factor) {
	case BLEND_FACTOR_ZERO: return GL_ZERO;
	case BLEND_FACTOR_ONE: return GL_ONE;
	case BLEND_FACTOR_SRC_COLOR: return GL_SRC_COLOR;
	case BLEND_FACTOR_INV_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
	case BLEND_FACTOR_SRC_ALPHA: return GL_SRC_ALPHA;
	case BLEND_FACTOR_INV_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
	case BLEND_FACTOR_DST_ALPHA: return GL_DST_ALPHA;
	case BLEND_FACTOR_INV_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
	case BLEND_FACTOR_DST_COLOR: return GL_DST_COLOR;
	case BLEND_FACTOR_INV_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
	case BLEND_FACTOR_SRC_ALPHA_SAT: return GL_SRC_ALPHA_SATURATE;
	}
	abort();
}

static void set_viewport(struct viewport *p)
{
	glViewport(p->x, p->y, p->w, p->h);
	glDepthRangef(p->zn, p->zf);
}

static void apply_fb_binding(struct fb_binding *p)
{
	int i;

	glBindFramebuffer(GL_FRAMEBUFFER, p->fbo);
	for (i = 0; i < p->nr_color_bindings; i++) {
		struct color_binding *binding = &p->color_bindings[i];

		switch (binding->action) {
		case FBO_CLEAR:
			glClearNamedFramebufferfv(p->fbo, GL_COLOR, i, (GLfloat *)&binding->color);
			break;
		case FBO_DONTCARE:
		case FBO_PRESERVE:
		default:
			break;
		}
	}
	if (p->enable_depth_stencil) {
		struct depth_stencil_binding *binding = &p->depth_stencil_binding;

		if (binding->depth_action == FBO_CLEAR && binding->stencil_action == FBO_CLEAR) {
			glClearNamedFramebufferfi(p->fbo, GL_DEPTH_STENCIL, 0, binding->depth, binding->stencil);
		} else {
			switch (binding->depth_action) {
			case FBO_CLEAR:
				glClearNamedFramebufferfv(p->fbo, GL_DEPTH, 0, &binding->depth);
				break;
			case FBO_PRESERVE:
			case FBO_DONTCARE:
			default:
				break;
			}
			switch (binding->stencil_action) {
			case FBO_CLEAR:
				glClearNamedFramebufferiv(p->fbo, GL_STENCIL, 0, (GLint *)&binding->stencil);
				break;
			case FBO_DONTCARE:
			case FBO_PRESERVE:
			default:
				break;
			}
		}
	}
}

static void set_raster_state(struct raster_state *p)
{
	if (p->cull_mode == CULL_OFF)
		glDisable(GL_CULL_FACE);
	else {
		glEnable(GL_CULL_FACE);
		glCullFace(p->cull_mode == CULL_BACK ? GL_BACK : GL_FRONT);
	}

	if (p->depth_clip)
		glEnable(GL_DEPTH_CLAMP);
	else
		glDisable(GL_DEPTH_CLAMP);

	if (equalf(p->offset_factor, 0.0f) && equalf(p->offset_units, 0.0f))
		glDisable(GL_POLYGON_OFFSET_FILL);
	else {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(p->offset_factor, p->offset_units);
	}
}

static void set_depth_stencil_state(struct depth_stencil_state *p)
{
	if (p->depth_enable) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(to_gl_depth_func(p->depth_compare_func));
		glDepthMask(p->depth_write ? GL_TRUE : GL_FALSE);
	} else
		glDisable(GL_DEPTH_TEST);
}

static void set_render_target_blend_state(int index, struct render_target_blend_state *p)
{
	if (p->blend_enable) {
		glEnablei(GL_BLEND, index);
		glBlendFuncSeparatei(index,
				     to_gl_blend_factor(p->src_color_factor),
				     to_gl_blend_factor(p->dst_color_factor),
				     to_gl_blend_factor(p->src_alpha_factor),
				     to_gl_blend_factor(p->dst_alpha_factor));
		glBlendEquationSeparatei(index, to_gl_blend_op(p->color_blend_op), to_gl_blend_op(p->alpha_blend_op));
	} else
		glDisablei(GL_BLEND, index);
	glColorMaski(index, (bool)p->write_red, (bool)p->write_green, (bool)p->write_blue, (bool)p->write_alpha);
}

static void set_all_blend_states(struct render_target_blend_state *p)
{
	if (p->blend_enable) {
		glEnable(GL_BLEND);
		glBlendFuncSeparate(to_gl_blend_factor(p->src_color_factor),
				    to_gl_blend_factor(p->dst_color_factor),
				    to_gl_blend_factor(p->src_alpha_factor),
				    to_gl_blend_factor(p->dst_alpha_factor));
		glBlendEquationSeparate(to_gl_blend_op(p->color_blend_op), to_gl_blend_op(p->alpha_blend_op));
	} else
		glDisable(GL_BLEND);
	glColorMask((bool)p->write_red, (bool)p->write_green, (bool)p->write_blue, (bool)p->write_alpha);
}

static void set_blend_state(struct blend_state *p)
{
	if (p->alpha_to_coverage)
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	else
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	if (p->independent_blend) {
		int i;

		for (i = 0; i < _countof(p->blend_states); i++)
			set_render_target_blend_state(i, &p->blend_states[i]);
	} else
		set_all_blend_states(&p->blend_states[0]);
}

static void set_uniform_1i(GLuint program, GLint location, int v)
{
	if (location != -1)
		glProgramUniform1i(program, location, v);
}

static void set_uniform_1f(GLuint program, GLint location, float v)
{
	if (location != -1)
		glProgramUniform1f(program, location, v);
}

static void set_uniform_3f(GLuint program, GLint location, float3 v)
{
	if (location != -1)
		glProgramUniform3f(program, location, v.x, v.y, v.z);
}

static void set_uniform_4f(GLuint program, GLint location, float4 v)
{
	if (location != -1)
		glProgramUniform4f(program, location, v.x, v.y, v.z, v.w);
}

static void set_uniform_4x4f(GLuint program, GLint location, float4x4 v)
{
	if (location != -1)
		glProgramUniformMatrix4fv(program, location, 1, GL_TRUE, v.data);
}

static void enable_attrib(GLuint index)
{
	if (index != -1)
		glEnableVertexAttribArray(index);
}

static void disable_attrib(GLuint index)
{
	if (index != -1)
		glDisableVertexAttribArray(index);
}

static void attrib_format(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
	if (attribindex != -1)
		glVertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
}

static void attrib_binding(GLuint attribindex, GLuint bindingindex)
{
	if (attribindex != -1)
		glVertexAttribBinding(attribindex, bindingindex);
}

static void change_shader(struct shader *from, struct shader *to)
{
	assert(from != to);

	if (!to) {
		disable_attrib(from->loc_position);
		disable_attrib(from->loc_normal);
		disable_attrib(from->loc_tangent);
		disable_attrib(from->loc_uv);
		glUseProgram(0);
		return;
	}

	/* Disable all attribs of the last program first. */
	if (from) {
		disable_attrib(from->loc_position);
		disable_attrib(from->loc_normal);
		disable_attrib(from->loc_tangent);
		disable_attrib(from->loc_uv);
	}

	glUseProgram(to->program);

	set_raster_state(&to->raster_state);
	set_depth_stencil_state(&to->depth_stencil_state);
	set_blend_state(&to->blend_state);

	enable_attrib(to->loc_position);
	enable_attrib(to->loc_normal);
	enable_attrib(to->loc_tangent);
	enable_attrib(to->loc_uv);

	attrib_format(to->loc_position, 3, GL_FLOAT, GL_FALSE, 0);
	attrib_format(to->loc_normal, 3, GL_FLOAT, GL_FALSE, 0);
	attrib_format(to->loc_tangent, 3, GL_FLOAT, GL_FALSE, 12);
	attrib_format(to->loc_uv, 2, GL_FLOAT, GL_FALSE, 0);

	attrib_binding(to->loc_position, SLOT_POSITION);
	attrib_binding(to->loc_normal, SLOT_NORTAN);
	attrib_binding(to->loc_tangent, SLOT_NORTAN);
	attrib_binding(to->loc_uv, SLOT_UV);

	if (to->loc_position != -1)
		glVertexBindingDivisor(SLOT_POSITION, 0);
	if (to->loc_normal != -1 || to->loc_tangent != -1)
		glVertexBindingDivisor(SLOT_NORTAN, 0);
	if (to->loc_uv != -1)
		glVertexBindingDivisor(SLOT_UV, 0);

	/* TODO: We should set these uniform locations once at shader creation time. */
	//set_uniform_1i(to->program, to->loc_sunlight_shadow_map, SLOT_SHADOW_MAP);
	set_uniform_1i(to->program, to->loc_albedo_map, SLOT_ALBEDO_MAP);
	set_uniform_1i(to->program, to->loc_normal_map, SLOT_NORMAL_MAP);
	set_uniform_1i(to->program, to->loc_specular_map, SLOT_SPECULAR_MAP);
	set_uniform_1i(to->program, to->loc_emissive_map, SLOT_EMISSIVE_MAP);
}

static void set_material(struct material *mat)
{
	struct shader *shdr = mat->shdr;

	if (shdr->loc_albedo_map != -1)
		glBindTextureUnit(SLOT_ALBEDO_MAP, mat->albedo_map->id);
	if (shdr->loc_normal_map != -1)
		glBindTextureUnit(SLOT_NORMAL_MAP, mat->normal_map->id);
	if (shdr->loc_specular_map != -1)
		glBindTextureUnit(SLOT_SPECULAR_MAP, mat->specular_map->id);
	if (shdr->loc_emissive_map != -1)
		glBindTextureUnit(SLOT_EMISSIVE_MAP, mat->emissive_map->id);
}

static void draw_mesh_part(struct mesh *p, int part_index, struct shader *shdr)
{
	struct mesh_part *part = &p->parts[part_index];

	/* We bind only buffers that the shader needs. */

	assert(shdr->loc_position != -1); /* We pretend all shaders using position attribute. */
	if ((shdr->loc_normal != -1 || shdr->loc_tangent != -1) && shdr->loc_uv != -1) {
		GLuint vbufs[MAX_VBUF_SLOTS] = {p->vertices.id, p->nortans.id, p->uvs.id};
		GLintptr offsets[MAX_VBUF_SLOTS] = {0, 0, 0};
		GLsizei strides[MAX_VBUF_SLOTS] = {p->vertices.stride, p->nortans.stride, p->uvs.stride};

		static_assert(SLOT_POSITION == 0 && SLOT_NORTAN == 1 && SLOT_UV == 2, "Fix this code please.");
		glBindVertexBuffers(0, _countof(vbufs), vbufs, offsets, strides);
	} else {
		if (shdr->loc_normal != -1 || shdr->loc_tangent != -1)
			glBindVertexBuffer(SLOT_NORTAN, p->nortans.id, 0, p->nortans.stride);
		if (shdr->loc_uv != -1)
			glBindVertexBuffer(SLOT_UV, p->uvs.id, 0, p->uvs.stride);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p->indices.id);
	assert(p->indices.stride == 2 || p->indices.stride == 4);
	glDrawElementsBaseVertex(
		GL_TRIANGLES,
		part->index_count,
		p->indices.stride == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
		(void *)(part->start_index * p->indices.stride),
		part->vertex_offset);
}

static void draw_elems_with_materials_and_light(int nr_elems, struct draw_element *elems, struct camera *cam, struct light *sunlight)
{
	float4x4 matrix_v = camera_matrix_v(cam);
	float4x4 matrix_p = camera_matrix_p(cam);
	float4x4 matrix_vp = camera_matrix_vp(cam);
	struct shader *prev_shdr = NULL;
	int i;

	glViewport(0, 0, scr_w, scr_h);
	apply_fb_binding(&main_fb_binding);

	for (i = 0; i < nr_elems; i++) {
		struct draw_element *elem = &elems[i];
		struct material *mat = elem->mat;
		struct shader *shdr = mat->shdr;

		if (shdr != prev_shdr) {
			change_shader(prev_shdr, shdr);
			prev_shdr = shdr;

			set_uniform_4x4f(shdr->program, shdr->loc_matrix_v, matrix_v);
			set_uniform_4x4f(shdr->program, shdr->loc_matrix_p, matrix_p);
			set_uniform_4x4f(shdr->program, shdr->loc_matrix_vp, matrix_vp);
			set_uniform_3f(shdr->program, shdr->loc_sunlight_dir, node_forward(&sunlight->node));
			set_uniform_3f(shdr->program, shdr->loc_sunlight_color, scale3f(sunlight->color, sunlight->intensity));
		}

		set_uniform_4x4f(shdr->program, shdr->loc_matrix_world, elem->matrix_world);
		set_uniform_4x4f(shdr->program, shdr->loc_matrix_inv_world, mat_inverse(elem->matrix_world));
		set_material(mat);
		draw_mesh_part(elem->mesh, elem->mesh_part, shdr);
	}

	if (prev_shdr)
		change_shader(prev_shdr, NULL);
}

void gfx_draw_scene(struct camera *cam, struct listnode *mesh_ent_list, struct listnode *light_list)
{
	thread_local struct draw_element elems[32768];
	int nr_elems = 0;
	struct light *sunlight = list_first_or_null(light_list, struct light, entry);
	struct mesh_entity *it;

	list_foreach(it, mesh_ent_list, struct mesh_entity, entry) {
		float4x4 matrix_world = node_wtm(&it->node);
		int i;

		for (i = 0; i < it->mesh->nr_parts; i++) {
			struct draw_element *elem = &elems[nr_elems++];

			elem->matrix_world = matrix_world;
			elem->mesh = it->mesh;
			elem->mesh_part = i;
			elem->mat = it->mesh->parts[i].mat;
		}
	}

	if (!sunlight || sunlight->type != LIGHT_DIRECTIONAL) {
		fprintf(stderr, "ERROR: There must be at least one light in the @list_list and the first light in the @light_list must be a directional light.");
		abort();
	}

	draw_elems_with_materials_and_light(nr_elems, elems, cam, sunlight);
}
