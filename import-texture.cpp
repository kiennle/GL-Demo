#include "core.h"
#include <stdlib.h>
#include <SOIL2.h>

struct texture *getorload_texture(const char *abs_path)
{
	struct texture *tex;

	tex = lookup_resource(abs_path, struct texture, rnode);
	if (tex)
		return tex;

	tex = (struct texture *)calloc(1, sizeof(*tex));
	if (!tex)
		return NULL;

	tex->id = SOIL_load_OGL_texture(abs_path, 0, 0, SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_LOAD_DIRECT);
	if (!tex->id) {
		free(tex);
		return NULL;
	}

	rnode_init(&tex->rnode, abs_path);
	rnode_add(&tex->rnode);
	return tex;
}
