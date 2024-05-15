#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const void *mapfile(const char *path, int *size);
void unmapfile(const void *map, int size);

#ifdef __cplusplus
}
#endif
