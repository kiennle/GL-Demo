#include "json.h"
#include "path.h"
#include "core.h"
#include "glutil.h"

static inline enum cull_mode string_to_cull_mode(const char *str)
{
	if (!strcmp(str, "off"))
		return CULL_OFF;
	else if (!strcmp(str, "back"))
		return CULL_BACK;
	else if (!strcmp(str, "front"))
		return CULL_FRONT;
	abort();
}

static inline enum depth_compare_func string_to_depth_compare_func(const char *str)
{
	if (!strcmp(str, "never"))
		return DEPTH_COMPARE_NEVER;
	else if (!strcmp(str, "less"))
		return DEPTH_COMPARE_LESS;
	else if (!strcmp(str, "equal"))
		return DEPTH_COMPARE_EQUAL;
	else if (!strcmp(str, "less_equal"))
		return DEPTH_COMPARE_LESS_EQUAL;
	else if (!strcmp(str, "greater"))
		return DEPTH_COMPARE_GREATER;
	else if (!strcmp(str, "not_equal"))
		return DEPTH_COMPARE_NOT_EQUAL;
	else if (!strcmp(str, "greater_equal"))
		return DEPTH_COMPARE_GREATER_EQUAL;
	else if (!strcmp(str, "always"))
		return DEPTH_COMPARE_ALWAYS;
	abort();
}

static inline enum blend_factor string_to_blend_factor(const char *str)
{
	if (!strcmp(str, "zero"))
		return BLEND_FACTOR_ZERO;
	else if (!strcmp(str, "one"))
		return BLEND_FACTOR_ONE;
	else if (!strcmp(str, "src_color"))
		return BLEND_FACTOR_SRC_COLOR;
	else if (!strcmp(str, "inv_src_color"))
		return BLEND_FACTOR_INV_SRC_COLOR;
	else if (!strcmp(str, "src_alpha"))
		return BLEND_FACTOR_SRC_ALPHA;
	else if (!strcmp(str, "inv_src_alpha"))
		return BLEND_FACTOR_INV_SRC_ALPHA;
	else if (!strcmp(str, "dst_alpha"))
		return BLEND_FACTOR_DST_ALPHA;
	else if (!strcmp(str, "inv_dst_alpha"))
		return BLEND_FACTOR_INV_DST_ALPHA;
	else if (!strcmp(str, "dst_color"))
		return BLEND_FACTOR_DST_COLOR;
	else if (!strcmp(str, "inv_dst_color"))
		return BLEND_FACTOR_INV_DST_COLOR;
	else if (!strcmp(str, "src_alpha_sat"))
		return BLEND_FACTOR_SRC_ALPHA_SAT;
	abort();
}

static inline enum blend_op string_to_blend_op(const char *str)
{
	if (!strcmp(str, "add"))
		return BLEND_OP_ADD;
	else if (!strcmp(str, "subtract"))
		return BLEND_OP_SUBTRACT;
	else if (!strcmp(str, "rev_subtract"))
		return BLEND_OP_REV_SUBTRACT;
	else if (!strcmp(str, "min"))
		return BLEND_OP_MIN;
	else if (!strcmp(str, "max"))
		return BLEND_OP_MAX;
	abort();
}

static void json_parse_raster_state(struct raster_state *p, const rapidjson::Value &jsval)
{
	p->cull_mode = string_to_cull_mode(jsval["cull_mode"].GetString());
	p->depth_clip = jsval["depth_clip"].GetBool();
	p->offset_factor = jsval["offset_factor"].GetFloat();
	p->offset_units = jsval["offset_units"].GetFloat();
	p->conservative_raster = jsval["conservative_raster"].GetBool();
}

static void json_parse_depth_stencil_state(struct depth_stencil_state *p, const rapidjson::Value &jsval)
{
	p->depth_enable = jsval["depth_enable"].GetBool();
	p->depth_write = jsval["depth_write"].GetBool();
	p->depth_compare_func = string_to_depth_compare_func(jsval["depth_compare_func"].GetString());
}

static void json_parse_blend_state(struct blend_state *p, const rapidjson::Value &jsval)
{
	int i;

	p->alpha_to_coverage = jsval["alpha_to_coverage"].GetBool();
	p->independent_blend = jsval["independent_blend"].GetBool();
	if (jsval["blend_states"].Size() == 0) /* require at least one blend state in array */
		abort();
	for (i = 0; i < jsval["blend_states"].Size(); i++) {
		const rapidjson::Value &blend_state = jsval["blend_states"][i];

		p->blend_states[i].blend_enable = blend_state["blend_enable"].GetBool();
		p->blend_states[i].src_color_factor = string_to_blend_factor(blend_state["src_color_factor"].GetString());
		p->blend_states[i].dst_color_factor = string_to_blend_factor(blend_state["dst_color_factor"].GetString());
		p->blend_states[i].color_blend_op = string_to_blend_op(blend_state["color_blend_op"].GetString());
		p->blend_states[i].src_alpha_factor = string_to_blend_factor(blend_state["src_alpha_factor"].GetString());
		p->blend_states[i].dst_alpha_factor = string_to_blend_factor(blend_state["dst_alpha_factor"].GetString());
		p->blend_states[i].alpha_blend_op = string_to_blend_op(blend_state["alpha_blend_op"].GetString());
		p->blend_states[i].write_red = blend_state["write_red"].GetBool();
		p->blend_states[i].write_green = blend_state["write_green"].GetBool();
		p->blend_states[i].write_blue = blend_state["write_blue"].GetBool();
		p->blend_states[i].write_alpha = blend_state["write_alpha"].GetBool();
	}
}

struct shader *getorload_shader(const char *abs_path)
{
	struct shader *shdr;
	rapidjson::Document doc;
	rapidjson::Value vertex_shader, fragment_shader;
	char vert_path[256], frag_path[256];

	shdr = lookup_resource(abs_path, struct shader, rnode);
	if (shdr)
		return shdr;

	shdr = (struct shader *)calloc(1, sizeof(*shdr));
	if (!shdr)
		return NULL;

	doc = parse_json(abs_path);
	vertex_shader = doc["vertex_shader"];
	fragment_shader = doc["fragment_shader"];
	resolve_path(vert_path, abs_path, vertex_shader.GetString());
	resolve_path(frag_path, abs_path, fragment_shader.GetString());

	json_parse_raster_state(&shdr->raster_state, doc["raster_state"]);
	json_parse_depth_stencil_state(&shdr->depth_stencil_state, doc["depth_stencil_state"]);
	json_parse_blend_state(&shdr->blend_state, doc["blend_state"]);

	shdr->program = gl_load_program(vert_path, frag_path);
	shdr->loc_position = glGetAttribLocation(shdr->program, "position");
	shdr->loc_normal = glGetAttribLocation(shdr->program, "normal");
	shdr->loc_tangent = glGetAttribLocation(shdr->program, "tangent");
	shdr->loc_uv = glGetAttribLocation(shdr->program, "uv");
	shdr->loc_matrix_world = glGetUniformLocation(shdr->program, "matrix_world");
	shdr->loc_matrix_inv_world = glGetUniformLocation(shdr->program, "matrix_inv_world");
	shdr->loc_matrix_v = glGetUniformLocation(shdr->program, "matrix_v");
	shdr->loc_matrix_p = glGetUniformLocation(shdr->program, "matrix_p");
	shdr->loc_matrix_vp = glGetUniformLocation(shdr->program, "matrix_vp");
	shdr->loc_sunlight_dir = glGetUniformLocation(shdr->program, "sunlight_dir");
	shdr->loc_sunlight_color = glGetUniformLocation(shdr->program, "sunlight_color");
	//shdr->loc_sunlight_matrix_vp = glGetUniformLocation(shdr->program, "sunlight_matrix_vp");
	//shdr->loc_sunlight_shadow_map = glGetUniformLocation(shdr->program, "sunlight_shadow_map");
	shdr->loc_albedo_map = glGetUniformLocation(shdr->program, "albedo_map");
	shdr->loc_normal_map = glGetUniformLocation(shdr->program, "normal_map");
	shdr->loc_specular_map = glGetUniformLocation(shdr->program, "specular_map");
	shdr->loc_emissive_map = glGetUniformLocation(shdr->program, "emissive_map");

	rnode_init(&shdr->rnode, abs_path);
	rnode_add(&shdr->rnode);
	return shdr;
}
