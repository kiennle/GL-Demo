#include "core.h"
#include <stdlib.h>

struct light *new_light()
{
	struct light *p = (struct light *)calloc(1, sizeof(*p));

	if (!p)
		return NULL;
	node_init(&p->node, NULL);
	p->type = LIGHT_DIRECTIONAL;
	p->color = float3_one;
	p->intensity = 4.0f;
	p->range = 10.0f;
	p->spot_angle = PI / 4.0f;
	listnode_init(&p->entry);
	return p;
}

void spawn_light(struct light *p)
{
	listnode_insert_before(&light_list, &p->entry);
}
