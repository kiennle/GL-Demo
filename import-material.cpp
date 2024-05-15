#include "json.h"
#include "path.h"
#include "core.h"

struct material *getorload_material(const char *abs_path)
{
	struct material *mat;
	rapidjson::Document doc;
	rapidjson::Value shader;
	rapidjson::Document::MemberIterator albedo_map;
	char tmp[256];

	mat = lookup_resource(abs_path, struct material, rnode);
	if (mat)
		return mat;

	mat = (struct material *)calloc(1, sizeof(*mat));
	if (!mat)
		return NULL;

	doc = parse_json(abs_path);
	
	shader = doc["shader"];
	resolve_path(tmp, abs_path, shader.GetString());
	mat->shdr = getorload_shader(tmp);

	albedo_map = doc.FindMember("albedo_map");
	if (albedo_map != doc.MemberEnd()) {
		resolve_path(tmp, abs_path, albedo_map->value.GetString());
		mat->albedo_map = getorload_texture(tmp);
	}

	rnode_init(&mat->rnode, abs_path);
	rnode_add(&mat->rnode);
	return mat;
}
