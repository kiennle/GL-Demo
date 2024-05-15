#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

const void *mapfile(const char *path, int *size)
{
	void *map = NULL;
	struct stat st;
	int err;
	int fd = open(path, O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "Failed to open '%s': %s\n", path, strerror(errno));
		return NULL;
	}

	err = fstat(fd, &st);
	if (err)
		goto out;
	*size = st.st_size;

	map = mmap(NULL, st.st_size, PROT_READ, 0, fd, 0);
	if (map == MAP_FAILED)
		map = NULL;
out:
	close(fd);
	return map;
}

void unmapfile(const void *map, int size)
{
	munmap((void *)map, size);
}
