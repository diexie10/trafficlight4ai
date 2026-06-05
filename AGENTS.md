# Repository Guidelines

## 项目结构与模块组织

本项目是一个面向 Linux 的 Qt 6 桌面工具，使用 CMake 和 C++17 构建。

- `src/` 存放主 GUI 程序和可复用核心逻辑。`tl4ai_core` 覆盖状态管理、配置和 IPC；`FloatingWindow`、`TrafficLightWidget`、`TrayIcon`、`SettingsDialog` 等 GUI 类也在这里。
- `tools/` 存放轻量级命令行客户端 `tl4ai-ctl`。
- `tests/` 存放基于 Qt Test 的测试程序，并通过 CTest 注册。
- `resources/` 存放 Qt 资源文件和交通灯图片。
- `docs/` 存放问题记录、设计说明和功能规格；`docs/superpowers/specs/` 是较早的功能设计目录。

## 构建、测试与开发命令

先安装 Qt 6 开发依赖，例如 Debian/Ubuntu：

```bash
sudo apt install qt6-base-dev qt6-multimedia-dev
```

配置并构建：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

本地运行 GUI：

```bash
./build/src/trafficlight4ai
```

构建后手动测试 CLI：

```bash
./build/tools/tl4ai-ctl red
./build/tools/tl4ai-ctl yellow
./build/tools/tl4ai-ctl green
```

运行测试：

```bash
cd build && ctest --output-on-failure
```

## 代码风格与命名约定

使用 C++17 和 Qt 惯用写法。缩进保持四个空格。类名使用 `PascalCase`；方法、局部变量和测试槽函数使用 `camelCase`；私有成员沿用现有 `m_` 前缀。优先使用清晰的 Qt 类型和 signal/slot 模式，不额外包装简单逻辑。新增 GUI 功能时保持设置对话框可预览、取消可回滚的现有交互模式。注释应简短，只解释代码本身不明显的行为。

## 测试指南

测试使用 Qt Test（`Qt6::Test`），并尽量通过 `tests/CMakeLists.txt` 中的 `add_tl4ai_test(...)` 添加。测试文件命名为 `test_<component>.cpp`，测试槽函数按被验证行为命名，例如 `duplicateCommandRefreshesTimeout`。修改状态流转、配置持久化、IPC、CLI 集成或提示音配置时，应补充聚焦测试。

## 提交与 Pull Request 规范

Git 历史使用简洁的 conventional-style 前缀，例如 `feat:`、`fix:`、`debug:`。提交标题应使用祈使语气并说明具体改动，例如 `fix: align default socket path fallback`。

Pull Request 应包含简短的问题说明、改动摘要、测试结果（`ctest --output-on-failure`），涉及可见 UI 变化时附截图或录屏。有关联 issue 或设计规格时一并链接。

## 安全与配置提示

不要提交个人配置文件、机器相关 socket 路径或本机音频文件路径。运行时配置位于 `~/.config/trafficlight4ai/config.json`；本地 socket 覆盖可使用 `TL4AI_SOCKET`。配置文件包含 `socket`、`animation`、`window`、`sound` 等字段；提示音为空时使用系统 beep。
