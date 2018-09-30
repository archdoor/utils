#include <stdlib.h>
#include <string.h>
#include "lj_threadpool.h"

void *threadpool_thread_func(void *args)
{
    lj_threadpool_t *threadpool = (lj_threadpool_t *)args;

    while ( 1 )
    {
        sem_wait(&threadpool->semaphor);

        pthread_mutex_lock(&threadpool->task_mutex);
        lj_threadpool_task_t *task = threadpool->task_list.next;
        if ( task == &threadpool->task_list )
        {
            pthread_mutex_unlock(&threadpool->task_mutex);
            continue;
        }
        else
        {
            threadpool->task_list.next = task->next;
            task->next->pre = &threadpool->task_list;
            pthread_mutex_unlock(&threadpool->task_mutex);
        }

        // 回掉函数
        threadpool->callback(task->data);

        free(task);
    }

    return NULL;
}

int lj_threadpool_start(lj_threadpool_t *threadpool)
{
    // 检查线程数配置
    if ( threadpool->thread_num <= 0 )
    {
        return -1;
    }

    // 任务列表初始化空
    threadpool->task_list.data = NULL;
    threadpool->task_list.next = &threadpool->task_list;
    threadpool->task_list.pre = &threadpool->task_list;

    // 互斥量初始化
    if ( pthread_mutex_init(&threadpool->task_mutex, NULL) != 0 )
    {
        return -1;
    }

    // 信号量初始化
    if ( sem_init(&threadpool->semaphor, 0, 0) != 0 )
    {
        return -1;
    }

    // 开启处理线程
    pthread_t tid = 0;
    for ( int i = 0; i < threadpool->thread_num; ++i )
    {
        if ( pthread_create(&tid, NULL, threadpool_thread_func, threadpool) != 0 )
        {
            return -1;
        }
        pthread_detach(tid);
    }

    return 0;
}

int lj_threadpool_push_data(lj_threadpool_t *threadpool, void *data)
{
    lj_threadpool_task_t *task = (lj_threadpool_task_t *)malloc(sizeof(lj_threadpool_task_t));

    task->data = data;
    task->next = &threadpool->task_list;
    task->pre = threadpool->task_list.pre;

    pthread_mutex_lock(&threadpool->task_mutex);
    threadpool->task_list.pre->next = task;
    threadpool->task_list.pre = task;

    pthread_mutex_unlock(&threadpool->task_mutex);

    sem_post(&threadpool->semaphor);

    return 0;
}



