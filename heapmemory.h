#ifndef __HEAP_MEMORY_H__
#define __HEAP_MEMORY_H__

#include <pthread.h>
#include <stdbool.h>

#define HEAPMEM_PAGE_MAX_SIZE 2048 // 只有该值可改动，同时需要改动全局量的初始化
#define HEAPMEM_PAGE_MIN_SIZE 64
#define HEAPMEM_PAGE_TYPE_NUM (HEAPMEM_PAGE_MAX_SIZE/HEAPMEM_PAGE_MIN_SIZE)

struct _heapmemory;

typedef struct _heapmem_page
{
    struct _heapmemory *heapmem;
    void *page;
    struct _heapmem_page *next;
    struct _heapmem_page *pre;
}heapmem_page_t;

typedef struct _heapmemory
{
    size_t count;
    heapmem_page_t busy_list;
    heapmem_page_t idle_list;
    pthread_mutex_t mutex;
}heapmemory_t;

extern heapmemory_t g_heapmem[];


void *heapmem_malloc(size_t size);
void *heapmem_calloc(size_t size);
void *heapmem_real_malloc(size_t size);
void *heapmem_real_calloc(size_t size);
void heapmem_free(void *mem);

void *heapmem_get_page(heapmemory_t *heapmem, size_t size, bool clean);


#endif



