#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

// 1. 包含系统头文件，解决pid_t未定义
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>  // 必须包含：pid_t的定义在这
#include <stdio.h>      // PRINT_ERROR/PRINT_INFO依赖

// 2. 前置声明C++类（避免和结构体重名）
#ifdef __cplusplus
class c_driver;
#else
// C语言中仅声明为指针类型，不定义结构体
typedef void* c_driver;
#endif


// 日志宏定义
#define PRINT_ERROR(fmt, ...) \
    do { \
        fprintf(stderr, "[错误] %d: " fmt "\n", __LINE__, ##__VA_ARGS__); \
    } while(0)

#define PRINT_INFO(fmt, ...) \
    do { \
        fprintf(stdout, "[正确] %d: " fmt "\n", __LINE__, ##__VA_ARGS__); \
    } while(0)

#endif // COMMON_TYPES_H