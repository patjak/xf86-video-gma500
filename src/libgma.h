#ifndef _LIBGMA_H
#define _LIBGMA_H

#include <uapi/drm/gma_drm.h>

#define ALIGN(x, y) (((x)+(y)-1) & ~((y)-1))

struct gma_bo {
	uint32_t handle;
	uint32_t size;
	uint32_t type;
	uint32_t pitch;
	void *ptr;
	int map_count;
};

struct gma_bo *gma_bo_create(int fd, uint32_t size, uint32_t type, uint32_t flags);
struct gma_bo *gma_bo_create_surface(int fd, uint32_t width, uint32_t height, uint32_t bpp, uint32_t type, uint32_t flags);
int gma_bo_mmap(int fd, struct gma_bo *bo);
int gma_bo_destroy(int fd, struct gma_bo *bo);

#endif
