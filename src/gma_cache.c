#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <xf86.h>
#include "libgma.h"
#include "gma_cache.h"

#define CACHE_SIZE 32

struct user_cache_entry {
	struct gma_bo *bo;
	uint32_t start;
	uint32_t end;
	uint32_t offset;
};

/* A cache of already wrapped userspace buffers */
static struct user_cache {
	struct user_cache_entry entries[CACHE_SIZE];
	uint32_t mem; /* Size of cache in bytes */
	int num; /* Number of entries */
	int rand; /* Gives psuedo randomness */
	int hits; /* Cache hits */
	int misses; /* Cache misses */
	int reduced; /* Entries reduced into other entries */
	int full; /* How many times the cache has been full */
} user_cache;

void user_cache_init(void)
{
	memset(&user_cache, 0, sizeof(struct user_cache));
}

void user_cache_del(int fd, struct user_cache_entry *entry)
{
	user_cache.mem -= entry->end - entry->start;
	gma_bo_destroy(fd, entry->bo);
	entry->bo = NULL;
	user_cache.num--;
}

void user_cache_add(int fd, struct gma_bo *bo, char *src, uint32_t size)
{
	struct user_cache_entry *entry; // = &user_cache[cache_pos];
	uint32_t start = (unsigned long)src;
	uint32_t end = size + (unsigned long)start;
	int i;

	/* Try reduce the cache first */
	for (i = 0; i < CACHE_SIZE; i++) {
		entry = &user_cache.entries[i];
		if (entry->bo && entry->start >= start && entry->end <= end) {
			user_cache_del(fd, entry);
			user_cache.reduced++;
		}
	}

	if (user_cache.num >= CACHE_SIZE) {
		entry = &user_cache.entries[user_cache.rand % CACHE_SIZE];
		user_cache_del(fd, entry);
		user_cache.full++;
	} else {
		/* Find a free spot */
		for (i = 0; i < CACHE_SIZE; i++) {
			entry = &user_cache.entries[i];
			if (!entry->bo)
				break;
		}
	}

	entry->bo = bo;
	entry->start = (unsigned long)src;
	entry->end = (unsigned long)src + size;
	entry->offset = bo->offset;
	user_cache.mem += entry->end - entry->start;
	user_cache.num++;

	// if (user_cache.mem >= 2 * 1024 * 1024)
		// ErrorF("Cache mem usage: %ukb\n", user_cache.mem / 1024);
}

struct gma_bo *user_cache_lookup(char *src, uint32_t size)
{
	uint32_t start = (unsigned long)src;
	uint32_t end = (unsigned long)src + size;
	struct user_cache_entry *entry;
	int i;

	user_cache.rand++;

	for (i = 0; i < CACHE_SIZE; i++) {
		entry = &user_cache.entries[i];
		if (entry->bo && start >= entry->start && end <= entry->end) {
			entry->bo->offset = (start - entry->start) + (entry->offset);
			user_cache.hits++;
			return entry->bo;
		}
	}

	user_cache.misses++;
	return NULL;
}
