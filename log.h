#ifndef __FWS_LOG_H__
#define __FWS_LOG_H__

#include <assert.h>
#include "zlog.h"

// 全局分类变量
extern zlog_category_t *g_cat;

#define LogInit(...)    zlog_init_with_category(__VA_ARGS__)

#define LogFini(...)    zlog_fini(__VA_ARGS__)

#define LogDebug(...)   zlog_debug(g_cat, __VA_ARGS__)
#define LogInfo(...)    zlog_info(g_cat, __VA_ARGS__)
#define LogNotice(...)  zlog_notice(g_cat, __VA_ARGS__)
#define LogWarn(...)    zlog_warn(g_cat, __VA_ARGS__)
#define LogError(...)   zlog_error(g_cat, __VA_ARGS__)
#define LogFatal(...)   zlog_fatal(g_cat, __VA_ARGS__)

// 内存十六进制打印
#define HLogDebug(...)   hzlog_debug(g_cat, __VA_ARGS__)
#define HLogInfo(...)    hzlog_info(g_cat, __VA_ARGS__)
#define HLogNotice(...)  hzlog_notice(g_cat, __VA_ARGS__)
#define HLogWarn(...)    hzlog_warn(g_cat, __VA_ARGS__)
#define HLogError(...)   hzlog_error(g_cat, __VA_ARGS__)
#define HLogFatal(...)   hzlog_fatal(g_cat, __VA_ARGS__)

// 断言定义
#define ASSERT(EXPR, TEXT) if ( !(EXPR) ) { LogFatal(TEXT); assert(0); } 

int zlog_init_with_category(const char *category, const char *confpath);

#endif

