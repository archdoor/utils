#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include "epoll_timer.h"

// 创建定时器集
eptimer_base *eptimer_base_new()
{
    eptimer_base *base = (eptimer_base *)calloc(1, sizeof(eptimer_base));
    if ( base == NULL )
    {
        return NULL;
    }

    if ( pthread_mutex_init(&base->mutex, NULL) != 0 )
    {
        free(base);
        return NULL;
    }

    return base;
}

// 创建定时器
eptimer *eptimer_new(int sec, int msec)
{
    eptimer *timer = (eptimer *)calloc(1, sizeof(eptimer));
    if ( timer == NULL )
    {
        return NULL;
    }

    timer->sec = sec;
    timer->msec = msec;

    return timer;
}

// 周期性定时器重新加入新的时间槽列表中
void eptimer_reset(eptimer_base *base, eptimer *timer)
{
    // 重新计算定时器归属时间槽
    timer->slot = (base->wheel.cur_slot + timer->offset) % EPTIMER_WHEEL_SLOT_NUM;

    // 将定时器加入相应时间槽的定时器列表
    timer->next = base->timer_list[timer->slot].next;
    timer->pre = &base->timer_list[timer->slot];
    if ( base->timer_list[timer->slot].next != NULL )
    {
        base->timer_list[timer->slot].next->pre = timer;
    }
    base->timer_list[timer->slot].next = timer;
}

// 执行当前时间槽上的定时器
void eptimer_run(eptimer_base *base)
{
    pthread_mutex_lock(&base->mutex);
    
    base->wheel.cur_slot += 1;
    if ( base->wheel.cur_slot == EPTIMER_WHEEL_SLOT_NUM )
    {
        base->wheel.cur_slot = 0;
    }

    eptimer *timer = base->timer_list[base->wheel.cur_slot].next;
    while ( timer != NULL )
    {
        timer->pre->next = timer->next;
        if ( timer->next != NULL )
        {
            timer->next->pre = timer->pre;
        }

        if ( timer->option & EPTIMER_PERSIST )
        {
            eptimer_reset(base, timer);
        }
        else
        {
            --base->size;
            timer->onwheel = 0;
        }

        timer->expire_cb(timer->args);

        timer = base->timer_list[base->wheel.cur_slot].next;
    }

    pthread_mutex_unlock(&base->mutex);
}

void *timer_base_thread_fn(void *args)
{
    eptimer_base *base = (eptimer_base *)args;

    struct epoll_event events;

    while ( epoll_wait(base->epfd, &events, 1, EPTIMER_MIN_MSEC) == 0 )
    {
        eptimer_run(base);
    }
    return NULL;
}

// 定时器集启动
int eptimer_base_start(eptimer_base *base)
{
    base->epfd = epoll_create(1000);
    if ( base->epfd == -1 )
    {
        return -1;
    }

    pthread_t tid;
    if ( pthread_create(&tid, NULL, timer_base_thread_fn, base) != 0 )
    {
        return -1;
    }
    return 0;
}

// 添加定时器
int eptimer_add(eptimer_base *base, EPTIMER_OPT opt, eptimer *timer, void (*expire_cb)(void *), void *args)
{
    // 检查定时器是否已经在时间轮上
    if ( timer->onwheel == 1 )
    {
        return -1;
    }
    
    // 计算定时器周期毫秒数
    int msec = timer->sec * 1000 + timer->msec;

    // 定时器时隙是否合法
    if ( (msec % EPTIMER_MIN_MSEC != 0) || (msec > g_max_slot) || (msec == 0) )
    {
        return -1;
    }

    // 检查定时器选项是否合法
    if ( (opt & EPTIMER_ONECE) && (opt & EPTIMER_PERSIST) )
    {
        return -1;
    }

    // 检查回调函数是否为空
    if ( expire_cb == NULL )
    {
        return -1;
    }

    // 是否立即执行
    if ( opt & EPTIMER_IMME )
    {
        expire_cb(args);
    }

    timer->option = opt;
    timer->expire_cb = expire_cb;
    timer->args = args;
    timer->offset = msec / EPTIMER_MIN_MSEC;    // 定时器偏移的时间槽数量

    // 加锁(整个定时器集)
    pthread_mutex_lock(&base->mutex);

    timer->slot = (base->wheel.cur_slot + timer->offset) % EPTIMER_WHEEL_SLOT_NUM;  // 计算定时器归属哪个时间槽

    // 将定时器加入相应时间槽的定时器列表
    timer->next = base->timer_list[timer->slot].next;
    timer->pre = &base->timer_list[timer->slot];
    if ( base->timer_list[timer->slot].next != NULL )
    {
        base->timer_list[timer->slot].next->pre = timer;
    }
    base->timer_list[timer->slot].next = timer;

    timer->onwheel = 1;

    // 计数
    ++base->size;

    // 解锁(整个定时器集)
    pthread_mutex_unlock(&base->mutex);

    return 0;
}

// 删除定时器
int eptimer_delete(eptimer_base *base, eptimer *timer)
{
    if ( timer->slot < 0 || timer->slot >= EPTIMER_WHEEL_SLOT_NUM )
    {
        return -1;
    }

    pthread_mutex_unlock(&base->mutex);

    if ( timer->onwheel == 0 )
    {
        pthread_mutex_unlock(&base->mutex);
        return 0;
    }

    timer->pre->next = timer->next;
    if ( timer->next != NULL )
    {
        timer->next->pre = timer->pre;
    }

    timer->onwheel = 0;

    // 计数
    --base->size;

    pthread_mutex_unlock(&base->mutex);

    return 0;
}

// 销毁定时器
int eptimer_free(eptimer *timer)
{
    // 检查定时器是否还在时间轮上
    if ( timer->onwheel == 1 )
    {
        return -1;
    }

    timer->onwheel = 0;

    free(timer);

    return 0;
}



