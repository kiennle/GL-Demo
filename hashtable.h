#pragma once
#include "list.h"

#ifndef countof
#define countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

/*

Simple optimized intrusive hash table implementation inspired by linux kernel hashtable (https://github.com/torvalds/linux/blob/master/include/linux/hashtable.h)
Usage:

struct object {
	int n;
	const char *name;
	struct hlistnode node;
};

void main() {
	struct object k1, k2, k3;
	struct bucket table[16];
	struct object *it;
	int bucket;

	k1.n = 1;
	k1.name = "Mario";
	k2.n = 2;
	k2.name = "Metroid";
	k3.n = 3;
	k3.name = "MegaMan";

	hashtable_init(table);
	hashtable_add(table, &k1.node, strhash(k1.name));
	hashtable_add(table, &k2.node, strhash(k2.name));
	hashtable_add(table, &k3.node, strhash(k3.name));

	hashtable_foreach(it, bucket, table, node) {
		printf("bucket = %d, n = %d\n", bucket, it->n);
	}

	hashtable_foreach_withkey(it, table, node, strhash("Metroid")) {
		printf("%d\n", it->n);
	}
}

*/

struct bucket {
	struct hlistnode *first;
};

static inline void bucket_add_first(struct bucket *bkt, struct hlistnode *node)
{
	struct hlistnode *first = bkt->first;

	node->next = first;
	if (first != NULL)
		first->pprev = &node->next;
	bkt->first = node;
	node->pprev = &bkt->first;
}

#define bucket_item(node, type, field) \
	(node) != NULL ? containerof(node, type, field) : NULL

#define bucket_foreach_node(ptr, bkt) \
	for (ptr = (bkt)->first; ptr != NULL; ptr = ptr->next)

#define bucket_foreach(item, bkt, type, field) \
	for (item = bucket_item((bkt)->first, type, field); \
	     item != NULL; \
	     item = bucket_item(item->field.next, type, field))

#define hashtable_init(ht)			\
do {						\
	unsigned int size = countof(ht);	\
	unsigned int i;				\
	for (i = 0; i < size; i++)		\
		ht[i].first = NULL;		\
} while(0)

/* Trả về bucket của một key. */
#define hashtable_bucket(ht, key) \
	(&ht[(key) & (countof(ht) - 1)])

#define hashtable_add(ht, node, key) \
	bucket_add_first(hashtable_bucket(ht, key), node)

/* @item và @idx(index của bucket) dùng làm loop cursor. */
#define hashtable_foreach(item, idx, ht, type, field) \
	for (idx = 0, item = NULL; item == NULL && idx < countof(ht); idx++) \
		bucket_foreach(item, &ht[idx], type, field)

/* @item dùng làm loop cursor. */
#define hashtable_foreach_withkey(item, key, ht, type, field) \
	bucket_foreach(item, hashtable_bucket(ht, key), type, field)
