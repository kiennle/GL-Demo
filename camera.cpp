#include "core.h"
#include <stdlib.h>

struct camera *new_camera()
{
	struct camera *p = (struct camera *)calloc(1, sizeof(*p));

	if (!p)
		return NULL;
	node_init(&p->node, NULL);
	p->ortho = false;
	p->fov = PI / 3.0f;
	p->aspect = 1366.0f / 768.0f;
	p->znear = 0.1f;
	p->zfar = 1000.0f;
	p->ortho_size = 768.0f / 2.0f;
	return p;
}

void del_camera(struct camera *p)
{
	node_deinit(&p->node);
	free(p);
}

float4x4 camera_matrix_v(struct camera *p)
{
	return node_matrix_v(&p->node);
}

float4x4 camera_matrix_p(struct camera *p)
{
	if (p->ortho) {
		float h = p->ortho_size * 2;
		float w = h * p->aspect;

		return mat_ortho(w, h, p->znear, p->zfar);
	}
	return mat_perspective(p->fov, p->aspect, p->znear, p->zfar);
}

float4x4 camera_matrix_vp(struct camera *p)
{
	return mat_mul(camera_matrix_v(p), camera_matrix_p(p));
}
