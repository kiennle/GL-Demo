#pragma once
#include <glad/glad.h>
#include "mathext.h"
#include "resdb.h"

enum fbo_action {
	FBO_DONTCARE,
	FBO_PRESERVE,
	FBO_CLEAR
};

struct color_binding {
	enum fbo_action action;
	float4 color;
};

struct depth_stencil_binding {
	enum fbo_action depth_action;
	float depth;
	enum fbo_action stencil_action;
	uint8_t stencil;
};

struct fb_binding {
	GLuint fbo;
	int nr_color_bindings;
	struct color_binding color_bindings[8];
	bool enable_depth_stencil;
	struct depth_stencil_binding depth_stencil_binding;
};

struct viewport {
	float x, y, w, h, zn, zf;
};

struct render_pass {
	struct fb_binding fb_binding;
	struct viewport viewport;
	struct shader *force_shdr; /* use this shader instead of shaders of draw elements */

	float4x4 matrix_v;
	float4x4 matrix_p;
	float4x4 matrix_vp;

	int nr_draw_items;
	struct draw_item *draw_items;

	struct render_pass *next; /* next render pass to execute */
};

enum cull_mode {
	CULL_OFF,
	CULL_BACK,
	CULL_FRONT
};

struct raster_state {
	enum cull_mode cull_mode;
	bool depth_clip;
	float offset_factor;
	float offset_units;
	bool conservative_raster; /* currently unused */
};

enum depth_compare_func {
	DEPTH_COMPARE_NEVER,
	DEPTH_COMPARE_LESS,
	DEPTH_COMPARE_EQUAL,
	DEPTH_COMPARE_LESS_EQUAL,
	DEPTH_COMPARE_GREATER,
	DEPTH_COMPARE_NOT_EQUAL,
	DEPTH_COMPARE_GREATER_EQUAL,
	DEPTH_COMPARE_ALWAYS
};

struct depth_stencil_state {
	bool depth_enable;
	bool depth_write;
	enum depth_compare_func depth_compare_func;
};

enum blend_op {
	BLEND_OP_ADD,
	BLEND_OP_SUBTRACT,
	BLEND_OP_REV_SUBTRACT,
	BLEND_OP_MIN,
	BLEND_OP_MAX
};

enum blend_factor {
	BLEND_FACTOR_ZERO,
	BLEND_FACTOR_ONE,
	BLEND_FACTOR_SRC_COLOR,
	BLEND_FACTOR_INV_SRC_COLOR,
	BLEND_FACTOR_SRC_ALPHA,
	BLEND_FACTOR_INV_SRC_ALPHA,
	BLEND_FACTOR_DST_ALPHA,
	BLEND_FACTOR_INV_DST_ALPHA,
	BLEND_FACTOR_DST_COLOR,
	BLEND_FACTOR_INV_DST_COLOR,
	BLEND_FACTOR_SRC_ALPHA_SAT
};

struct render_target_blend_state {
	bool blend_enable;
	enum blend_factor src_color_factor;
	enum blend_factor dst_color_factor;
	enum blend_op color_blend_op;
	enum blend_factor src_alpha_factor;
	enum blend_factor dst_alpha_factor;
	enum blend_op alpha_blend_op;
	bool write_red;
	bool write_green;
	bool write_blue;
	bool write_alpha;
};

struct blend_state {
	bool alpha_to_coverage;
	bool independent_blend;
	struct render_target_blend_state blend_states[8];
};

struct shader {
	struct res_node rnode;

	GLuint program;
	struct raster_state raster_state;
	struct depth_stencil_state depth_stencil_state;
	struct blend_state blend_state;

	/* attribs */
	GLint loc_position;
	GLint loc_normal;
	GLint loc_tangent;
	GLint loc_uv;

	/* object uniforms */
	GLint loc_matrix_world;
	GLint loc_matrix_inv_world;

	/* view uniforms */
	GLint loc_matrix_v;
	GLint loc_matrix_p;
	GLint loc_matrix_vp;

	/* light uniforms */
	GLint loc_sunlight_dir;
	GLint loc_sunlight_color; /* color is already multiplied by intensity */

	//GLint loc_sunlight_matrix_vp;
	//GLint loc_sunlight_shadow_map; /* sunlight shadow texture map */

	/* material uniforms */
	GLint loc_albedo_map;
	GLint loc_normal_map;
	GLint loc_specular_map;
	GLint loc_emissive_map;
};

struct texture {
	struct res_node rnode;
	GLuint id;
};

struct material {
	struct res_node rnode;
	struct shader *shdr;
	struct texture *albedo_map;
	struct texture *normal_map;
	struct texture *specular_map;
	struct texture *emissive_map;
};

struct mesh_part {
	struct material *mat;
	int vertex_offset;
	int start_index;
	int index_count;
};

struct gfx_buffer {
	int stride;
	int count;
	void *cpu_data;
	GLuint id;
};

struct nortan {
	float3 normal, tangent;
};

struct mesh {
	struct res_node rnode;

	int nr_parts;
	struct mesh_part *parts;

	struct gfx_buffer vertices; /* float3 */
	struct gfx_buffer nortans; /* float3-float3 */
	struct gfx_buffer uvs; /* float2 */
	struct gfx_buffer blends; /* float4-byte4 */
	struct gfx_buffer indices; /* uint32 */
};

struct scn_node {
	/* Node metadata: node type, name, sequence tracks, etc. */
	//struct mdx_node *mdx_node;

	/* NULL if this is the root node in the hierarchy. */
	struct scn_node *parent;

	/*
	* Sorted by parent, parents must come first its children.
	* We should update anim state of nodes in this order.
	* NULL for last node.
	*/
	struct scn_node *next;
	unsigned short nr_descendants;

	unsigned short preserved;
	unsigned int flags;

	float3 pos; /* in world space */
	float4 rot; /* in world space */
	float3 scl; /* in world space */
};

struct camera {
	struct scn_node node;
	bool ortho; /* use orthographics or perspective projection */
	float fov;
	float aspect;
	float znear;
	float zfar;
	float ortho_size; /* half height (like fovy), used for ortho camera */
};

enum light_type {
	LIGHT_POINT,
	LIGHT_SPOT,
	LIGHT_DIRECTIONAL
};

struct light {
	struct scn_node node;
	enum light_type type;
	float3 color;
	float intensity;
	float range; /* used for point light and spot light */
	float spot_angle; /* used for spot light */
	struct listnode entry; /* renderer keeps track this light */
};

struct mesh_entity {
	struct scn_node node;
	struct mesh *mesh;
	struct listnode entry; /* renderer keeps track this mesh */
};

extern struct camera *main_cam;
extern struct listnode mesh_ent_list; /* list of struct mesh_entity */
extern struct listnode light_list; /* list of struct light */

void node_init(struct scn_node *p, struct mdx_node *mdx_node);
void node_deinit(struct scn_node *p);
struct scn_node *new_node(struct mdx_node *mdx_node);
void del_node(struct scn_node *p);
void node_add_child(struct scn_node *p, struct scn_node *child);
void node_leave(struct scn_node *p);
void node_reparent(struct scn_node *p, struct scn_node *parent);
float3 node_forward(struct scn_node *p);
float3 node_up(struct scn_node *p);
float3 node_right(struct scn_node *p);
float3 node_down(struct scn_node *p);
float3 node_left(struct scn_node *p);
float3 node_backward(struct scn_node *p);
float4x4 node_wtm(struct scn_node *p);
float4x4 node_matrix_v(struct scn_node *p);

struct camera *new_camera();
void del_camera(struct camera *p);
float4x4 camera_matrix_v(struct camera *p);
float4x4 camera_matrix_p(struct camera *p);
float4x4 camera_matrix_vp(struct camera *p);

struct light *new_light();
void spawn_light(struct light *p);

struct mesh_entity *new_mesh_entity(struct mesh *mesh);
void spawn_mesh_entity(struct mesh_entity *p);

struct texture *getorload_texture(const char *abs_path);
struct shader *getorload_shader(const char *abs_path);
struct material *getorload_material(const char *abs_path);
struct mesh *getorload_mesh(const char *abs_path);
void load_scene(const char *path);

void gfx_init(float width, float height);
void gfx_draw_scene(struct camera *cam, struct listnode *mesh_ent_list, struct listnode *light_list);





