#include "json.h"
#include "path.h"
#include "core.h"
#include <string>

static void json_parse_node(struct scn_node *p, const rapidjson::Value &jsval)
{
	p->pos.x = jsval["pos"][0].GetFloat();
	p->pos.y = jsval["pos"][1].GetFloat();
	p->pos.z = jsval["pos"][2].GetFloat();
	p->rot.x = jsval["rot"][0].GetFloat();
	p->rot.y = jsval["rot"][1].GetFloat();
	p->rot.z = jsval["rot"][2].GetFloat();
	p->rot.w = jsval["rot"][3].GetFloat();
	p->scl.x = jsval["scl"][0].GetFloat();
	p->scl.y = jsval["scl"][1].GetFloat();
	p->scl.z = jsval["scl"][2].GetFloat();
}

static void parse_entity(const char *scn_path, const rapidjson::Value &ent)
{
	std::string type = ent["type"].GetString();

	if (type == "camera") {
		struct camera *p = new_camera();

		json_parse_node(&p->node, ent["scn_node"]);
		p->ortho = ent["ortho"].GetBool();
		p->aspect = ent["aspect"].GetFloat();
		p->fov = ent["fov"].GetFloat();
		p->znear = ent["znear"].GetFloat();
		p->zfar = ent["zfar"].GetFloat();
		p->ortho_size = ent["ortho_size"].GetFloat();
		if (!main_cam)
			main_cam = p;
	} else if (type == "mesh_entity") {
		struct mesh_entity *p;
		char mesh_path[256];

		resolve_path(mesh_path, scn_path, ent["mesh"].GetString());
		p = new_mesh_entity(getorload_mesh(mesh_path));
		json_parse_node(&p->node, ent["scn_node"]);
		spawn_mesh_entity(p);
	} else if (type == "light") {
		struct light *p;
		std::string light_type;

		p = new_light();
		json_parse_node(&p->node, ent["scn_node"]);
		light_type = ent["light_type"].GetString();
		if (light_type == "directional")
			p->type = LIGHT_DIRECTIONAL;
		else if (light_type == "point")
			p->type = LIGHT_POINT;
		else if (light_type == "spot")
			p->type = LIGHT_SPOT;
		else
			abort();
		p->color.x = ent["color"][0].GetFloat();
		p->color.y = ent["color"][1].GetFloat();
		p->color.z = ent["color"][2].GetFloat();
		p->intensity = ent["intensity"].GetFloat();
		p->range = ent["range"].GetFloat();
		p->spot_angle = ent["spot_angle"].GetFloat();
		spawn_light(p);
	} else
		printf("WARNING: Unknown entity type \"%s\" while parsing scene.", type.data());
}

void load_scene(const char *path)
{
	rapidjson::Document doc;
	rapidjson::Value entities;
	rapidjson::Value::ConstValueIterator it;

	doc = parse_json(path);
	entities = doc["entities"];
	for (it = entities.Begin(); it != entities.End(); ++it)
		parse_entity(path, *it);
}
