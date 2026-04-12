# Application.mk（路径：asfbaiofwuwghf/Application.mk）
APP_STL := c++_static          # 明确指定静态C++库
APP_ABI := arm64-v8a           # 编译架构
APP_PLATFORM := android-35     # 目标Android版本
APP_FORCE_STATIC_EXECUTABLE := true  # 全局强制静态编译
APP_ALLOW_MISSING_DEPS := true # 忽略未找到的依赖（兜底）