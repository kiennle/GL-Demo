#pragma once
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static inline bool is_abs_path(const char *path)
{
	return path[0] == '/';
}

static inline bool is_local_path(const char *path)
{
	return path[0] != '/';
}

/* @relpath is relative to @local_filepath. */
static inline void make_path_abs(char *rel_path, const char *local_filepath)
{
	char drive[8], dir[256], buf[256];

	_splitpath(local_filepath, drive, dir, NULL, NULL);
	sprintf(buf, "%s%s%s", drive, dir, rel_path);
	strcpy(rel_path, buf);
}

static inline void resolve_path(char *dst, const char *base_path, const char *rel_path)
{
	char drive[8], dir[256], buf[256];

	_splitpath(base_path, drive, dir, NULL, NULL);
	sprintf(buf, "%s%s%s", drive, dir, rel_path);
	strcpy(dst, buf);
	if (access(dst, F_OK) != 0) /* file doesn't exist */
		strcpy(dst, rel_path); /* @rel_path should be an absolute path */
}

///*
// * Resolve to an absolute path.
// * See: https://stackoverflow.com/questions/39110801/path-join-vs-path-resolve-with-dirname
// */
//static inline void resolve_path(char *dst, const char *root_path, const char *path)
//{
//
//}
