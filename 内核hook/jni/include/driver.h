#ifndef DRIVER_H
#define DRIVER_H
// C/C++兼容宏
#ifdef __cplusplus
#include <cstdint>
#include <vector>
#include <string>
#else
#include <stdint.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>
class c_driver {
private:
    int fd;
    pid_t pid;

    // IOCTL操作码
    enum OPERATIONS {
        OP_INIT_KEY = 0x800,
        OP_READ_MEM = 0x801,
        OP_WRITE_MEM = 0x802,
        OP_MODULE_BASE = 0x803,
        OP_HIDE_PROCESS = 0x804

    };

    // 内部结构体
    typedef struct {
        pid_t pid;
        uintptr_t addr;
        void* buffer;
        size_t size;
    } COPY_MEMORY;

    typedef struct {
        pid_t pid;
        char* name;
        uintptr_t base;
    } MODULE_BASE;

    // 内部方法
    char* driver_path();

public:
    // 构造/析构
    c_driver();
    ~c_driver();


void initialize(pid_t pid);



bool init_key(char* key);


bool read(uintptr_t addr, void* buffer, size_t size);


bool write(uintptr_t addr, void* buffer, size_t size);


uintptr_t get_module_base(char* name);
};

#endif

// 2. 全局驱动实例（C/C++兼容）
#ifdef __cplusplus
extern c_driver* g_driver;
#else
extern c_driver g_driver;
#endif

// 3. 工具函数声明（C/C++兼容）
#ifdef __cplusplus
extern "C" {
#endif
pid_t get_pid_by_package(const char* package);
uintptr_t get_so_base(pid_t pid, const char* so_name);

}

