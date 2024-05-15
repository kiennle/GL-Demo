#include "core.h"
#include <stdlib.h>

struct mesh_entity *new_mesh_entity(struct mesh *mesh)
{
	struct mesh_entity *p = (struct mesh_entity *)calloc(1, sizeof(*p));

	if (!p)
		return NULL;
	node_init(&p->node, NULL);
	p->mesh = mesh;
	listnode_init(&p->entry);
	return p;
}

void spawn_mesh_entity(struct mesh_entity *p)
{
	listnode_insert_before(&mesh_ent_list, &p->entry);
}
