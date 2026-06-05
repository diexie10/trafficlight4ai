# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

trafficlight4ai 是一个 C++ Qt6 桌面应用，为 AI 编码工具（Codex、Claude Code 等）提供可视化红绿灯状态指示器。

- **红灯闪烁**：AI 工具正在工作中
- **黄灯闪烁**：AI 工具等待用户确认
- **绿灯常亮**：工作完成

展现形式：系统托盘图标 + 可拖动悬浮小窗口（默认左上角）。

## 技术栈

- **语言**：C++17
- **GUI 框架**：Qt 6（Core, Widgets, Network, Multimedia, Test）
- **构建系统**：CMake（最低 3.20）
- **目标平台**：Linux
- **状态检测**：AI 工具 Hooks → tl4ai-ctl CLI → Unix Domain Socket
- **国际化**：Qt Linguist（QTranslator + .ts/.qm），支持英语/中文/日语

## 构建命令

```bash
# 配置
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build build -j$(nproc)

# 运行 GUI
./build/src/trafficlight4ai

# 运行全部测试
cd build && ctest --output-on-failure

# 运行单个测试
cd build && ctest -R test_state_manager --output-on-failure

# Release 构建
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(nproc)
```

## 项目目录结构

```
trafficlight4ai/
├── CMakeLists.txt                 # 根构建文件
├── CLAUDE.md                      # 项目指南
├── docs/superpowers/specs/        # 设计文档
├── src/
│   ├── CMakeLists.txt             # tl4ai_core 静态库 + trafficlight4ai 可执行文件
│   ├── StateManager.h/cpp         # 状态机（纯逻辑，含超时机制）
│   ├── ConfigManager.h/cpp        # JSON 配置管理
│   ├── IpcServer.h/cpp            # Unix Domain Socket 服务端
│   ├── AiToolStrategy.h           # AI 工具策略接口 + Registry
│   ├── TrafficLightWidget.h/cpp   # 红绿灯绘制控件
│   ├── FloatingWindow.h/cpp       # 可拖动悬浮窗口
│   ├── TrayIcon.h/cpp             # 系统托盘图标
│   ├── SettingsDialog.h/cpp       # 设置对话框（实时预览+取消撤销）
│   ├── SoundUtils.h/cpp           # 音效播放工具（QMediaPlayer + beep fallback）
│   └── main.cpp                   # 入口
├── tests/
│   ├── CMakeLists.txt
│   ├── test_state_manager.cpp     # StateManager 单元测试
│   ├── test_config_manager.cpp    # ConfigManager 单元测试
│   ├── test_ipc_server.cpp        # IPC 协议集成测试
│   ├── test_ai_tool_strategy.cpp  # AiToolStrategy 单元测试
│   └── test_tl4ai_ctl.cpp         # CLI 集成测试
├── tools/
│   ├── CMakeLists.txt
│   └── tl4ai_ctl.cpp              # 纯 POSIX CLI，不依赖 Qt
├── translations/
│   ├── trafficlight4ai_zh.ts      # 中文翻译
│   └── trafficlight4ai_ja.ts      # 日语翻译
└── resources/
    ├── resources.qrc
    └── images/                    # 红绿灯 PNG 图片
```

## 架构

### 两个可执行文件

- **trafficlight4ai**（`src/`）— Qt GUI 主进程，内嵌 IPC Server
- **tl4ai-ctl**（`tools/`）— 纯 POSIX 轻量 CLI，不依赖 Qt，通过 socket 发送状态指令

### 核心类（`src/`）

| 类 | 职责 | 依赖 |
|---|------|------|
| `StateManager` | 状态机（Working/WaitingConfirm/Idle），超时自动回 Idle | Qt Core |
| `ConfigManager` | JSON 配置读写（`~/.config/trafficlight4ai/config.json`） | Qt Core |
| `IpcServer` | QLocalServer 监听 Unix Domain Socket，解析指令转发给 StateManager | Qt Network |
| `AiToolStrategy` | 策略接口，封装各 AI 工具的差异（hooks 模板、默认超时等） | 无 |
| `TrafficLightWidget` | 自定义 QWidget，绘制三灯 UI，支持呼吸灯/经典闪烁 | Qt Widgets |
| `FloatingWindow` | 无边框置顶窗口，可拖动，记忆位置 | Qt Widgets |
| `TrayIcon` | 系统托盘图标，颜色随状态变化，右键菜单 | Qt Widgets |
| `SettingsDialog` | 设置对话框，实时预览配置变更，取消可撤销 | Qt Widgets |
| `SoundUtils` | 音效播放（QMediaPlayer，支持 WAV/MP3/OGG，fallback 系统 beep） | Qt Multimedia |

### 数据流

```
AI Tool Hooks → tl4ai-ctl (POSIX CLI) → Unix Domain Socket → IpcServer → StateManager → TrafficLightWidget + TrayIcon
```

### 库分层

`tl4ai_core`（静态库）包含 StateManager、ConfigManager、IpcServer，供测试和主程序共用。GUI 组件仅在主程序中链接。

### Socket 协议

默认路径：`$XDG_RUNTIME_DIR/trafficlight4ai.sock`（fallback `/tmp/trafficlight4ai-$UID.sock`）。可通过 config.json 或 `TL4AI_SOCKET` 环境变量覆盖。

指令为单行纯文本：`RED\n` / `YELLOW\n` / `GREEN\n`，大小写不敏感，无法识别的指令静默忽略。

### AI 工具策略模式

`AiToolStrategy` 接口封装不同 AI 工具的差异，通过 `AiToolRegistry` 注册和查找：
- `CodexStrategy` — Codex hooks 配置模板
- `ClaudeCodeStrategy` — Claude Code hooks 配置模板

新增工具只需添加一个 Strategy 实现并注册到 Registry。

## 测试

使用 Qt Test（QTest），每个测试为独立可执行文件：

| 测试 | 覆盖范围 |
|------|---------|
| `test_state_manager` | 状态切换、signal 发射、命令解析、超时机制 |
| `test_config_manager` | 默认值、读写持久化、容错回退、参数校验、aiTool/timeoutSec |
| `test_ipc_server` | socket 收发、无效指令、旧 socket 清理、restart |
| `test_ai_tool_strategy` | 事件名验证、hooks 模板内容、Registry 查找 |
| `test_tl4ai_ctl` | CLI 集成测试，需要编译后的 tl4ai-ctl 二进制 |

## 代码约定

- 类名 PascalCase，方法和变量 camelCase
- Qt 信号槽使用新式语法（`&Class::signal`）
- 头文件使用 `#pragma once`
- 私有成员变量前缀 `m_`
- 所有 UI 文本使用 `tr()` 包裹（国际化）
