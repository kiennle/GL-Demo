#pragma once
#include <stddef.h>

/*
 * This header contains a lot of unrelated to each
 * other stuff that does not fit anywhere else.
 */

#ifndef countof
#define countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

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
