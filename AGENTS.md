# Repository Guidelines

## 项目结构与模块组织

本项目是 Qt 6 桌面工具，使用 CMake 和 C++17 构建。`src/` 包含 GUI 主程序与核心逻辑：`tl4ai_core` 覆盖 `StateManager`、`ConfigManager`、`IpcServer`，`TrafficLightWidget`、`FloatingWindow`、`TrayIcon`、`SettingsDialog`、`SoundUtils` 负责界面、托盘、设置和提示音。`tools/` 提供基于 Qt `QLocalSocket` 的 CLI `tl4ai-ctl`。`tests/` 是 Qt Test/CTest 测试。`resources/images/` 存放交通灯 PNG，`translations/` 存放中文和日语 `.ts` 翻译，`docs/` 存放问题记录和设计说明。

## 构建、测试与开发命令

Ubuntu/Debian 依赖：

```bash
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-tools-dev
```

常用命令：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug      # 配置调试构建
cmake --build build -j$(nproc)               # 编译 GUI、CLI、测试
./build/src/trafficlight4ai                  # 启动 GUI
./build/tools/tl4ai-ctl red                  # 手动发送状态指令
cd build && ctest --output-on-failure        # 运行全部测试
cd build && ctest -R test_state_manager --output-on-failure
```

Release 构建可使用 `cmake -B build-release -DCMAKE_BUILD_TYPE=Release`。Debug 构建保留 `qDebug()` 开发日志；Release 构建定义 `QT_NO_DEBUG_OUTPUT`，调试级日志会在编译时移除，但 warning 和 critical 错误仍会输出。

Windows 构建由 `.github/workflows/windows-build.yml` 负责，使用 MSVC、`-DBUILD_TESTING=OFF` 和 `windeployqt` 生成 `trafficlight4ai-windows-x86_64.zip` artifact。Windows GUI 主程序必须保持 `WIN32_EXECUTABLE TRUE`，避免启动时弹出控制台窗口；`tl4ai-ctl` 仍作为 CLI 工具发布。

## 代码风格与命名约定

使用四空格缩进、C++17 和 Qt 惯用写法。类名用 `PascalCase`，方法、变量和测试槽函数用 `camelCase`，私有成员使用 `m_` 前缀，头文件使用 `#pragma once`。Qt signal/slot 使用新式连接语法。所有可见 UI 文本必须用 `tr()` 包裹，并同步更新 `translations/trafficlight4ai_zh.ts` 和 `translations/trafficlight4ai_ja.ts`。新增 GUI 设置应保持实时预览、取消可回滚的现有行为。

## 测试指南

测试基于 Qt Test，并通过 `tests/CMakeLists.txt` 注册。新增测试优先使用 `add_tl4ai_test(test_<component>)`；CLI 集成测试需要编译后的 `tl4ai-ctl`。测试文件命名为 `test_<component>.cpp`，测试槽函数按行为命名，例如 `duplicateCommandRefreshesTimeout`。修改状态流转、配置读写、IPC、CLI、窗口尺寸、动画、国际化或提示音时，应补充聚焦测试。

## 文档同步

`README.md` 是英文文档，`README_zh.md` 是对应的中文文档。更新构建方式、功能说明、配置字段、运行行为或用户可见命令时，必须同时更新两份 README，保持内容一致。仅修复某一语言的表达问题时，可只改对应文件。

## 配置与资源注意事项

运行配置位于 `~/.config/trafficlight4ai/config.json`，socket 路径或名称可用 `TL4AI_SOCKET` 覆盖。当前配置包含 `language`、`aiTool`、`timeoutSec`、`window.size`（`xsmall`/`small`/`medium`/`large`/`xlarge`）、`animation.mode`、`animation.periodMs`、`socket.path` 和 `sound.*`。不要提交个人配置、机器相关 socket 路径或本机音频路径。新增图片请放入 `resources/images/` 并在 Qt 资源配置中注册。

## 发布注意事项

GitHub Release 应同时上传 Ubuntu tar.gz、Windows zip 和 `SHA256SUMS.txt`。Ubuntu 包来自本地 Release 构建并需通过 `ctest --test-dir build-release --output-on-failure`；Windows 包来自成功的 `Windows Build` artifact。`dist/` 是本地发布产物目录，已被 `.gitignore` 忽略，不要提交。

## 提交与 Pull Request 规范

Git 历史使用简洁的 conventional-style 前缀，如 `feat:`、`fix:`、`docs:`、`debug:`。标题使用祈使语气并说明具体改动，例如 `fix: resize floating window geometry after size changes`。Pull Request 应包含问题说明、改动摘要和测试结果；涉及可见 UI、图片、窗口行为或动画变化时附截图或录屏，并链接相关 issue 或设计文档。
