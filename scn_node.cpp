#include "core.h"
#include <stdlib.h>
#include <assert.h>

void node_init(struct scn_node *p, struct mdx_node *mdx_node)
{
	p->parent = NULL;
	p->next = NULL;
	p->nr_descendants = 0;
	p->preserved = 0;
	p->flags = 0;
	p->pos = float3_zero;
	p->rot = quat_identity;
	p->scl = float3_one;
}

void node_deinit(struct scn_node *p)
{
}

struct scn_node *new_node(struct mdx_node *mdx_node)
{
	struct scn_node *p = (struct scn_node *)calloc(1, sizeof(*p));

	node_init(p, mdx_node);
	return p;
}

void del_node(struct scn_node *p)
{
	node_deinit(p);
	free(p);
}

static struct scn_node *node_get_last(struct scn_node *p)
{
	for (; p->next; p = p->next);
	return p;
}

static struct scn_node *node_last_descendant(struct scn_node *p)
{
	struct scn_node *tmp;
	int i;

	for (i = 0, tmp = p; i < p->nr_descendants; i++, tmp = tmp->next);
	return tmp;
}

static struct scn_node *node_get_prev(struct scn_node *p)
{
	struct scn_node *tmp;

	if (!p->parent)
		return NULL;
	for (tmp = p->parent; tmp->next != p; tmp = tmp->next);
	return tmp;
}

void node_add_child(struct scn_node *p, struct scn_node *child)
{
	struct scn_node *tmp;

	assert(!child->parent);
	child->parent = p;
	node_get_last(child)->next = p->next;
	p->next = child;
	for (tmp = p; tmp; tmp = tmp->parent)
		tmp->nr_descendants += child->nr_descendants;
}

void node_leave(struct scn_node *p)
{
	struct scn_node *last_descendant = node_last_descendant(p);
	struct scn_node *prev = node_get_prev(p);
	struct scn_node *tmp;

	for (tmp = p->parent; tmp; tmp = tmp->parent)
		tmp->nr_descendants -= p->nr_descendants;
	if (prev)
		prev->next = last_descendant->next;
	last_descendant->next = NULL;
	p->parent = NULL;
}

void node_reparent(struct scn_node *p, struct scn_node *parent)
{
	node_leave(p);
	if (parent)
		node_add_child(parent, p);
}

float3 node_forward(struct scn_node *p)
{
	return quat_forward(p->rot);
}

float3 node_up(struct scn_node *p)
{
	return quat_up(p->rot);
}

float3 node_right(struct scn_node *p)
{
	return quat_right(p->rot);
}

float3 node_down(struct scn_node *p)
{
	return quat_down(p->rot);
}

float3 node_left(struct scn_node *p)
{
	return quat_left(p->rot);
}

float3 node_backward(struct scn_node *p)
{
	return quat_backward(p->rot);
}

float4x4 node_wtm(struct scn_node *p)
{
	return mat_transform(p->pos, p->rot, p->scl);
}

float4x4 node_matrix_v(struct scn_node *p)
{
	return mat_lookto(p->pos, node_forward(p), float3_up);
}
