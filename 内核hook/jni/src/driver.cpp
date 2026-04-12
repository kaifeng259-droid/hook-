// ========== 核心头文件包含 ==========
#include "../include/driver.h"  // 包含c_driver类定义
#include "../include/common_types.h"  // 包含内存拷贝等结构体定义
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctime>
#include <unistd.h>
#include <ctype.h>

using namespace std;

// 全局驱动实例
c_driver* g_driver = nullptr;

// ================== 补全缺失的核心逻辑 ==================

/**
 * 初始化驱动：记录目标进程 PID
 */
void c_driver::initialize(pid_t pid) {
    this->pid = pid;
    PRINT_INFO("驱动已与目标进程关联: %d", pid);
}

/**
 * 核心方法：读取内存
 * 通过 ioctl 向内核驱动发送 OP_READ_MEM 指令
 */
bool c_driver::read(uintptr_t addr, void* buffer, size_t size) {
    if (fd < 0 || pid < 0 || !buffer || size == 0) return false;

    COPY_MEMORY cm = {0};
    cm.pid = pid;      // 目标进程
    cm.addr = addr;    // 目标地址
    cm.buffer = buffer;// 缓冲区
    cm.size = size;    // 大小

    // 调用驱动 IO 控制接口
    return ioctl(fd, OP_READ_MEM, &cm) == 0;
}

/**
 * 核心方法：写入内存 (Patch)
 * 通过 ioctl 向内核驱动发送 OP_WRITE_MEM 指令
 */
bool c_driver::write(uintptr_t addr, void* buffer, size_t size) {
    if (fd < 0 || pid < 0 || !buffer || size == 0) return false;

    COPY_MEMORY cm = {0};
    cm.pid = pid;      // 目标进程
    cm.addr = addr;    // 目标地址
    cm.buffer = buffer;// 写入的数据内容
    cm.size = size;    // 大小

    // 调用驱动 IO 控制接口
    return ioctl(fd, OP_WRITE_MEM, &cm) == 0;
}

// ================== 原有驱动查找逻辑 ==================

// 内部方法：查找驱动路径
char* c_driver::driver_path() {
    PRINT_INFO("正在 /dev 目录中搜索随机命名的驱动设备...");

    const char* dev_path = "/dev";
    DIR* dir = opendir(dev_path);
    if (!dir) {
        PRINT_ERROR("打开dev目录失败: %m");
        return nullptr;
    }

    const char* excluded_names[] = {"binder", "common", "ashmem", "stdin", "stdout", "stderr", nullptr};
    struct dirent* entry;
    char* file_path = nullptr;

    while ((entry = readdir(dir)) != nullptr) {
        const char* name = entry->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        if (strstr(name, "gpiochip") != nullptr) continue;
        if (strchr(name, '_') != nullptr || strchr(name, '-') != nullptr || strchr(name, ':') != nullptr) continue;

        int is_excluded = 0;
        for (int i = 0; excluded_names[i]; i++) {
            if (strcmp(name, excluded_names[i]) == 0) {
                is_excluded = 1;
                break;
            }
        }
        if (is_excluded) continue;

        size_t path_len = strlen(dev_path) + strlen(name) + 2;
        file_path = (char*)malloc(path_len);
        snprintf(file_path, path_len, "%s/%s", dev_path, name);

        struct stat st;
        if (stat(file_path, &st) != 0) {
            free(file_path);
            file_path = nullptr;
            continue;
        }

        if (!S_ISCHR(st.st_mode) && !S_ISBLK(st.st_mode)) {
            free(file_path);
            file_path = nullptr;
            continue;
        }

        if (localtime(&st.st_ctime)->tm_year + 1900 <= 1980) {
            free(file_path);
            file_path = nullptr;
            continue;
        }

        if (st.st_atime == st.st_ctime && st.st_size == 0 && st.st_gid == 0 && st.st_uid == 0 && strlen(name) == 6) {
            closedir(dir);
            PRINT_INFO("匹配到潜在驱动设备: %s", file_path);
            return file_path;
        }

        free(file_path);
        file_path = nullptr;
    }

    closedir(dir);
    return nullptr;
}

// 构造函数
c_driver::c_driver() : fd(-1), pid(-1) {
    char* dev_path = driver_path();
    if (dev_path) {
        fd = open(dev_path, O_RDWR);
        free(dev_path);
        if (fd >= 0) {
            PRINT_INFO("驱动已打开成功 (fd: %d)", fd);
        } else {
            PRINT_ERROR("无法打开驱动设备，请检查是否已加载驱动或具备Root权限");
        }
    } else {
        PRINT_ERROR("错误：未在系统中找到符合特征的驱动设备");
        exit(EXIT_FAILURE);
    }
}

// 析构函数
c_driver::~c_driver() {
    if (fd >= 0) {
        close(fd);
        PRINT_INFO("驱动已安全关闭");
    }
}

