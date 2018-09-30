#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "heapmemory.h"
#include "log.h"

heapmemory_t g_heapmem[HEAPMEM_PAGE_TYPE_NUM] = {
    [0] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [1] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [2] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [3] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [4] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [5] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [6] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [7] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [8] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [9] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [10] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [11] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [12] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [13] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [14] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [15] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [16] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [17] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [18] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [19] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [20] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [21] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [22] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [23] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [24] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [25] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [26] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [27] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [28] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [29] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [30] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
    [31] = { .mutex = PTHREAD_MUTEX_INITIALIZER, },
};

void *heapmem_malloc(size_t size)
{
    for ( int i = 0; i < HEAPMEM_PAGE_TYPE_NUM; ++i )
    {
        if ( size <= (HEAPMEM_PAGE_MIN_SIZE * (i + 1)) )
        {
            return heapmem_get_page(&g_heapmem[i], HEAPMEM_PAGE_MIN_SIZE * (i + 1), false);
        }
    }
    return NULL;
}

void *heapmem_calloc(size_t size)
{
    for ( int i = 0; i < HEAPMEM_PAGE_TYPE_NUM; ++i )
    {
        if ( size <= (HEAPMEM_PAGE_MIN_SIZE * (i + 1)) )
        {
            return heapmem_get_page(&g_heapmem[i], HEAPMEM_PAGE_MIN_SIZE * (i + 1), true);
        }
    }
    return NULL;
}

void *heapmem_real_malloc(size_t size)
{
    void *mem = heapmem_malloc(size);
    while ( mem == NULL )
    {
        mem = heapmem_malloc(size);
        usleep(1000);
    }
    return mem;
}

void *heapmem_real_calloc( size_t size)
{
    void *mem = heapmem_calloc(size);
    while ( mem == NULL )
    {
        mem = heapmem_calloc(size);
        usleep(1000);
    }
    return mem;
}

void heapmem_free(void *mem)
{
    if ( mem == NULL )
        return;

    heapmem_page_t *node = (heapmem_page_t *)(mem - sizeof(heapmem_page_t));
    heapmemory_t *heapmem = node->heapmem;

    pthread_mutex_lock(&heapmem->mutex);

    heapmem->busy_list.next = node->next;
    if ( node->next != NULL )
    {
        node->next->pre = &heapmem->busy_list;
    }

    node->next = heapmem->idle_list.next;
    node->pre = &heapmem->idle_list;
    if ( heapmem->idle_list.next != NULL )
    {
        heapmem->idle_list.next->pre = node;
    }
    heapmem->idle_list.next = node;

    pthread_mutex_unlock(&heapmem->mutex);
}

void *heapmem_get_page(heapmemory_t *heapmem, size_t size, bool clean)
{
    pthread_mutex_lock(&heapmem->mutex);

    size_t node_size = sizeof(heapmem_page_t) + size + 1;

    heapmem_page_t *node = heapmem->idle_list.next;
    if ( node == NULL )
    {
        node = malloc(node_size);
        if ( node == NULL )
        {
            pthread_mutex_unlock(&heapmem->mutex);
            return NULL;
        }
        heapmem->count++;
        if ( !(heapmem->count % 20) )
        {
            LogDebug("HeapMem[%d]:%d", size, heapmem->count);
        }

        node->heapmem = heapmem;
        node->page = ((void *)node) + sizeof(heapmem_page_t);
    }
    else
    {
        heapmem->idle_list.next = node->next;
        if ( node->next != NULL )
        {
            node->next->pre = &heapmem->idle_list;
        }
    }

    node->next = heapmem->busy_list.next;
    node->pre = &heapmem->busy_list;
    if ( heapmem->busy_list.next != NULL )
    {
        heapmem->busy_list.next->pre = node;
    }
    heapmem->busy_list.next = node;

    pthread_mutex_unlock(&heapmem->mutex);

    if ( clean )
    {
        memset(node->page, 0, size);
    }

    return node->page;
}


