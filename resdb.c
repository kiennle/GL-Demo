#include <string.h>
#include "hash.h"
#include "resdb.h"

static struct bucket res_dict[MAX_RES_ENTRIES];

void rnode_init(struct res_node *p, const char *name)
{
	hlistnode_init(&p->entry);
	strncpy(p->name, name, sizeof(p->name));
}

void rnode_add(struct res_node *p)
{
	hashtable_add(res_dict, &p->entry, strihash(p->name)); /* case-insensitive */
}

void rnode_remove(struct res_node *p)
{
	hlistnode_leave(&p->entry);
}

struct res_node *rnode_lookup(const char *name)
{
	struct res_node *p;
	unsigned int key = strihash(name); /* case-insensitive */

	hashtable_foreach_withkey(p, key, res_dict, struct res_node, entry)
		if (!stricmp(p->name, name))
			return p;
	return NULL;
}
