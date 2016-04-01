#ifndef __BH_MALLOC_H
#define __BH_MALLOC_H

void* bh_malloc(int size);
void bh_free(void* ptr);
void* bh_realloc(void* ptr, int newsize);

#endif
