LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# 模块名称
LOCAL_MODULE := HWTSS.sh
LOCAL_MODULE_TAGS := optional

# ========== 1. 拆分C/C++编译选项 ==========
LOCAL_CFLAGS := -std=c99 -Wno-error=format-security -w -fpermissive -fvisibility=hidden -ffunction-sections -fdata-sections
LOCAL_CPPFLAGS := -std=c++17 -Wno-error=format-security -fpermissive -w -Werror -s -fno-rtti -fno-exceptions -fms-extensions -Wno-error=c++11-narrowing -fvisibility=hidden

# 宏定义
LOCAL_CFLAGS += -DCONFIG_HW_BREAKPOINT_MODE=1 -D__STDC_FORMAT_MACROS

# ========== 2. 头文件路径 ==========
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

# ========== 3. 源码文件 ==========
LOCAL_SRC_FILES := src/main.cpp
LOCAL_SRC_FILES += src/driver.cpp


# ========== 4. 链接参数（核心修复：移除c++_static） ==========
LOCAL_LDLIBS := -ldl  # 仅保留dl库
LOCAL_LDFLAGS := -static -Wl,--strip-all  # 静态编译，无PIE
LOCAL_FORCE_STATIC_EXECUTABLE := true    # 强制静态可执行文件

# 编译可执行文件
include $(BUILD_EXECUTABLE)