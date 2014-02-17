#ifndef _LIBGMA_H
#define _LIBGMA_H

#include <uapi/drm/gma_drm.h>

#define ALIGN(x, y) (((x)+(y)-1) & ~((y)-1))

struct gma_bo;

struct gma_blit_op {
	struct gma_bo *src_bo;
	struct gma_bo *dst_bo;
	struct gma_bo *mask_bo;
	uint32_t fill_color;
	uint32_t direction;
	uint32_t rotation;
	uint32_t src_fmt;
	uint32_t dst_fmt;
	uint32_t mask_fmt;
	uint32_t src_stride;
	uint32_t dst_stride;
	int rop;
};

struct gma_bo {
	struct gma_blit_op blit_op;
	uint32_t handle;
	uint32_t offset;
	uint32_t size;
	uint32_t type;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t bpp;
	void *ptr;
	int map_count;
	int wrapped;
};

struct gma_bo *gma_bo_create(int fd, uint32_t size, uint32_t type, uint32_t flags);
struct gma_bo *gma_bo_create_surface(int fd, uint32_t width, uint32_t height, uint32_t bpp, uint32_t type, uint32_t flags);
struct gma_bo *gma_bo_wrap(int fd, char *ptr, uint32_t size);
int gma_bo_mmap(int fd, struct gma_bo *bo);
int gma_bo_destroy(int fd, struct gma_bo *bo);
int gma_blt_submit(int fd, uint32_t handle, uint32_t size);

#endif
