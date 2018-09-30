#ifndef __UTILS_THREADPOOL_H__
#define __UTILS_THREADPOOL_H__

#include <pthread.h>
#include <semaphore.h>

typedef struct _lj_threadpool_task_t
{
    void *data;
    struct _lj_threadpool_task_t *next;
    struct _lj_threadpool_task_t *pre;
}lj_threadpool_task_t;


typedef struct
{
    int thread_num;
    void (*callback)(void *);

    lj_threadpool_task_t task_list;
    pthread_mutex_t task_mutex;
    sem_t semaphor;
}lj_threadpool_t;



int lj_threadpool_start(lj_threadpool_t *threadpool);

int lj_threadpool_push_data(lj_threadpool_t *threadpool, void *data);


#endif



