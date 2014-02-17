#ifndef _GMA_CACHE_H
#define _GMA_CACHE_H

struct user_cache_entry;

void user_cache_init(void);
void user_cache_add(int fd, struct gma_bo *bo, char *src, uint32_t size);
void user_cache_del(int fd, struct user_cache_entry *entry);
struct gma_bo *user_cache_lookup(char *src, uint32_t size);

#endif
