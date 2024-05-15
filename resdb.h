#pragma once
#include "hashtable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RNODE_NAME_LEN 128

struct res_node {
	struct hlistnode entry;
	/*
	 * @name is case-insensitive.
	 * Trick: We can use file extension to detect resource type.
	 */
	char name[MAX_RNODE_NAME_LEN];
};

#define RES_NODE_INIT {.entry = HLISTNODE_INIT, .name = ""}
#define MAX_RES_ENTRIES 65536

void rnode_init(struct res_node *p, const char *name);
void rnode_add(struct res_node *p);
void rnode_remove(struct res_node *p);
struct res_node *rnode_lookup(const char *name);
#define lookup_resource(name, type, field) containerof_safe(rnode_lookup(name), type, field)

#ifdef __cplusplus
}
#endif
