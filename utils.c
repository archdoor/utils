#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

// 获取当前可执行文件路径
void get_exe_path(char *name, int len)
{
    readlink("/proc/self/exe", name, len);  

    char *end = strrchr(name, '/');

    *(end + 1) = '\0';
}

// 时间转化成字符串 2018-09-01 12:10:08.13245
void date_to_string(struct timeval *time, char *sdate)
{
    char buff[64] = {0};
    struct tm dc = {0};
    localtime_r(&time->tv_sec, &dc);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &dc);
    sprintf(sdate, "%s.%.6ld", buff, time->tv_usec);
}

