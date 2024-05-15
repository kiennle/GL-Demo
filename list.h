#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifndef OFFSETOF
/* https://en.wikipedia.org/wiki/Offsetof#Implementation */
#define OFFSETOF(st, m) \
	((size_t)((char *)&((st *)0)->m - (char *)0))
#endif

#ifndef containerof

#define containerof(ptr, type, field) \
	((type *)( (char *)(ptr) - OFFSETOF(type, field) ))

static inline void *_containerof_safe(void *ptr, size_t offset)
{
	return ptr ? (char *)ptr - offset : NULL;
}

/* Similar to containerof but return NULL if @ptr is NULL. */
#define containerof_safe(ptr, type, field) \
	( (type *)_containerof_safe(ptr, OFFSETOF(type, field)) )
#endif

struct listnode {
	struct listnode *next, *prev;
};

/*
 * @next equals to NULL if this is the last node of hlist.
 * @pprev can be NULL too.
 */
struct hlistnode {
	struct hlistnode *next, **pprev;
};

#define LISTNODE_INIT(head) {.next = head, .prev = head}
#define HLISTNODE_INIT {.next = NULL, .pprev = NULL}

/* Initialize a list node. */
static inline void
listnode_init(struct listnode *node)
{
	node->next = node;
	node->prev = node;
}

static inline bool listnode_is_first(struct listnode *sentinel, struct listnode *node)
{
	return sentinel->next == node;
}

static inline bool listnode_is_last(struct listnode *sentinel, struct listnode *node)
{
	return sentinel->prev == node;
}

static inline bool list_empty(struct listnode *sentinel)
{
	return sentinel->next == sentinel;
}

static inline struct listnode *list_first_node_or_null(struct listnode *sentinel)
{
	return list_empty(sentinel) ? NULL : sentinel->next;
}

static inline struct listnode *list_last_node_or_null(struct listnode *sentinel)
{
	return list_empty(sentinel) ? NULL : sentinel->prev;
}

#define list_first(sentinel, type, field) \
	containerof((sentinel)->next, type, field)

#define list_last(sentinel, type, field) \
	containerof((sentinel)->prev, type, field)

#define list_first_or_null(sentinel, type, field) \
	containerof_safe(list_first_node_or_null(sentinel), type, field)

#define list_last_or_null(sentinel, type, field) \
	containerof_safe(list_last_node_or_null(sentinel), type, field)

/* Insert a node before the specified one. */
static inline void
listnode_insert_before(struct listnode *node,
		       struct listnode *inserted)
{
	struct listnode *left = node->prev;
	struct listnode *middle = inserted;
	struct listnode *right = node;

	left->next = middle;
	middle->prev = left;
	middle->next = right;
	right->prev = middle;
}

/* Insert a node after the specified one. */
static inline void
listnode_insert_after(struct listnode *node,
		      struct listnode *inserted)
{
	struct listnode *left = node;
	struct listnode *middle = inserted;
	struct listnode *right = node->next;

	left->next = middle;
	middle->prev = left;
	middle->next = right;
	right->prev = middle;
}

/* Leave the list this node belongs to. */
static inline void
listnode_leave(struct listnode *node)
{
	struct listnode *left = node->prev;
	struct listnode *right = node->next;

	left->next = right;
	right->prev = left;
}

/* Check if a list node is not in any list. */
static inline bool
listnode_alone(const struct listnode *node)
{
	return node->next == node && node->prev == node;
}

#define list_foreach_node(cursor, sentinel) \
	for (cursor = (sentinel)->next; cursor != (sentinel); cursor = cursor->next)

#define list_foreach(item, sentinel, type, field) \
	for (item = containerof((sentinel)->next, type, field);	\
	     &item->field != (sentinel); \
	     item = containerof(item->field.next, type, field))

#define list_foreach_rev(item, sentinel, type, field) \
	for (item = containerof((sentinel)->prev, type, field); \
	     &item->field != (sentinel); \
	     item = containerof(item->field.prev, type, field))

static inline void
hlistnode_init(struct hlistnode *node)
{
	node->next = NULL;
	node->pprev = NULL;
}

static inline void
hlistnode_insert_before(struct hlistnode *node,
			struct hlistnode *inserted)
{
	inserted->pprev = node->pprev;
	inserted->next = node;
	node->pprev = &inserted->next;
	*(inserted->pprev) = inserted;
}

static inline void
hlistnode_insert_after(struct hlistnode *node,
		       struct hlistnode *inserted)
{
	inserted->next = node->next;
	node->next = inserted;
	inserted->pprev = &node->next;
	if (inserted->next)
		inserted->next->pprev = &inserted->next;
}

/* Leave the list this node belongs to. */
static inline void hlistnode_leave(struct hlistnode *node)
{
	struct hlistnode **left = node->pprev;
	struct hlistnode *right = node->next;

	*left = right;
	if (right)
		right->pprev = left;
}

static inline bool hlistnode_alone(struct hlistnode *node)
{
	return !node->next && !node->pprev;
}
