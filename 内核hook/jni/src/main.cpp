#include "../include/driver.h"
#include "../include/common_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h> // 用于 isdigit

// ================= 配置参数 (Configuration) =================
#define TARGET_PACKAGE "com.tencent.tmgp.dfm" // 目标游戏包名
#define TARGET_SO      "libtersafe.so"       // 欲修改的动态库名称
#define TARGET_OFFSET  0x562C40              // 想要修改的代码在 SO 里的偏移地址

// ================= 工具函数 (Utility Functions) =================

/**
 * 根据包名获取进程 PID
 * 遍历 /proc 目录下的所有进程 cmdline 文件
 */
pid_t get_pid_by_package(const char* package) {
    DIR* dir = opendir("/proc"); // 打开系统进程目录
    if (!dir) return -1;

    struct dirent* entry;
    char path[256], buf[256];
    pid_t pid = -1;

    while ((entry = readdir(dir)) != nullptr) {
        // 如果目录名不是数字，说明不是进程目录，跳过
        if (!isdigit(entry->d_name[0])) continue;

        // 构建 cmdline 文件路径，例如 /proc/1234/cmdline
        snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);

        FILE* fp = fopen(path, "r");
        if (fp) {
            memset(buf, 0, sizeof(buf));
            fread(buf, 1, sizeof(buf) - 1, fp); // 读取进程包名
            fclose(fp);

            // 如果读取到的包名包含目标字符串，记录 PID 并退出
            if (strstr(buf, package) != nullptr) {
                pid = atoi(entry->d_name);
                break;
            }
        }
    }
    closedir(dir);
    return pid;
}

/**
 * 获取进程中指定 SO 模块的起始基地址
 * 读取 /proc/[pid]/maps 获取内存布局
 */
uintptr_t get_so_base(pid_t pid, const char* so_name) {
    char maps_path[256];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    FILE* fp = fopen(maps_path, "r");
    if (!fp) return 0;

    char line[1024];
    uintptr_t base_addr = 0;
    while (fgets(line, sizeof(line), fp)) {
        // 在每一行内存映射信息中查找目标 SO 名字
        if (strstr(line, so_name) != nullptr) {
            // 找到第一行匹配的地址，即为基地址 (起始地址)
            base_addr = strtoul(line, nullptr, 16);
            break;
        }
    }
    fclose(fp);
    return base_addr;
}

// ================= 核心修改逻辑 (Core Hook/Patch Logic) =================

void perform_direct_patch() {
    // 1. 获取目标进程 PID (Find the Target PID)
    pid_t pid = get_pid_by_package(TARGET_PACKAGE);
    if (pid == -1) {
        PRINT_ERROR("错误: 未找到正在运行的目标进程: %s", TARGET_PACKAGE);
        return;
    }

    // 2. 获取目标库基址 (Find the base address of SO)
    uintptr_t target_so_base = get_so_base(pid, TARGET_SO);
    if (target_so_base == 0) {
        PRINT_ERROR("错误: 在进程内未找到模块: %s", TARGET_SO);
        return;
    }

    // 3. 初始化驱动 (Connect driver to the target process)
    g_driver->initialize(pid);

    // 4. 计算需要修改的真实内存地址 (Absolute Address = Base + Offset)
    uintptr_t absolute_addr = target_so_base + TARGET_OFFSET;

    PRINT_INFO("目标进程 PID: %d", pid);
    PRINT_INFO("模块基址: 0x%lx", target_so_base);
    PRINT_INFO("修改目标: 0x%lx", absolute_addr);

    // 5. 准备要写入的内容 (Data to patch)
    // 假设我们要把此处指令改为 ARM64 的 RET (返回) 指令
    // 机器码 0xD65F03C0 代表 "ret"
    uint32_t patch_code = 0xD65F03C0;

    // 6. 执行内存写入 (Write the patch into memory)
    // 这里使用驱动提供的 write 接口，强行覆盖目标地址的内容
    if (g_driver->write(absolute_addr, &patch_code, sizeof(patch_code))) {
        PRINT_INFO("修改成功! 已将地址 0x%lx 处的指令替换。", absolute_addr);
    } else {
        PRINT_ERROR("写入失败! 请确认驱动权限或目标地址是否有效。");
    }

    // 7. 清理资源 (Cleanup)
    PRINT_INFO("操作完成，正在释放资源...");
    delete g_driver;
    exit(0);
}

// ================= 程序入口 (Main Entry) =================

int main(int argc, char* argv[]) {
    // 实例化驱动对象
    g_driver = new c_driver();

    printf("\n======= 驱动hook修改工具 (Memory Patch Tool) =======\n");
    printf("目标包名: %s\n", TARGET_PACKAGE);
    printf("目标模块: %s\n", TARGET_SO);
    printf("----------------------------------------------------\n");
    printf("1. 执行地址修改 (Patch Address)\n");
    printf("0. 退出 (Exit)\n");
    printf("请选择: ");

    char choice_buf[32];
    if (fgets(choice_buf, sizeof(choice_buf), stdin)) {
        if (atoi(choice_buf) == 1) {
            perform_direct_patch(); // 调用核心逻辑
        }
    }

    delete g_driver;
    return 0;
}