#ifndef __EPOLL_TIMER_H__
#define __EPOLL_TIMER_H__

#include <pthread.h>

// 时间轮槽数
#define EPTIMER_WHEEL_SLOT_NUM (24 * 3600 * 10)
// 最小定时器时隙(单位：毫秒)
#define EPTIMER_MIN_MSEC 100
// 最大定时器时隙(24小时)
#define g_max_slot (EPTIMER_WHEEL_SLOT_NUM * EPTIMER_MIN_MSEC)

// 定时器属性
typedef enum
{
    EPTIMER_ONECE = 0x01,      // 单次定时器
    EPTIMER_PERSIST = 0x02,    // 永久定时器
    EPTIMER_IMME = 0x04,    // 立刻执行
}EPTIMER_OPT;

// 时间轮
typedef struct
{
    int cur_slot;   // 当前时间槽
}eptimer_wheel;

// 定时器
typedef struct _epoll_timer
{
    int option; // 定时器选项(是否单次定时器)
    int sec;    // 定时器周期秒数
    int msec;   // 定时器周期毫秒数
    void (*expire_cb)(void *);   // 定时器处理函数
    void *args; // 定时器处理函数参数
    int offset; // 定时器偏移的时间槽数量
    int slot;   // 定时器所属时间槽
    int onwheel;    // 定时器是否在时间轮上

    struct _epoll_timer *next;
    struct _epoll_timer *pre;
}eptimer;

// 定时器集
typedef struct
{
    eptimer_wheel wheel;    // 时间轮
    eptimer timer_list[EPTIMER_WHEEL_SLOT_NUM];   // 时间轮各个时间槽上定时器列表
    int epfd;   // epoll句柄
    pthread_mutex_t mutex; // 整个定时器集互斥量
    int size;   // 时间轮定时器总数
}eptimer_base;



// 创建定时器集
eptimer_base *eptimer_base_new();
// 创建定时器
eptimer *eptimer_new(int sec, int msec);
// 定时器集启动
int eptimer_base_start(eptimer_base *base);
// 添加定时器
int eptimer_add(eptimer_base *base, EPTIMER_OPT opt, eptimer *timer, void (*expire_cb)(void *), void *args);
// 删除定时器
int eptimer_delete(eptimer_base *base, eptimer *timer);
// 销毁定时器
int eptimer_free(eptimer *timer);

#endif



