# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

trafficlight4ai 是一个 C++ Qt6 桌面应用，为 AI 编码工具（Codex、Claude Code、Qoder CN、Copilot 等）提供可视化红绿灯状态指示器。

- **红灯闪烁**：AI 工具正在工作中
- **黄灯闪烁**：AI 工具等待用户确认
- **绿灯常亮**：工作完成

展现形式：系统托盘图标 + 可拖动悬浮小窗口（默认左上角）。

## 技术栈

- **语言**：C++17
- **GUI 框架**：Qt 6（Core, Widgets, Network, Multimedia, LinguistTools, Test）
- **构建系统**：CMake（最低 3.20）
- **目标平台**：Linux，Windows（最低 Windows 10）
- **状态检测**：AI 工具 Hooks → tl4ai-ctl CLI → Qt local IPC socket
- **国际化**：Qt Linguist（QTranslator + .ts/.qm），支持英语/中文/日语

## 构建与验证

Linux 和 Windows 的编译前提、编译命令、注意事项与验证方式统一维护在 [docs/BUILD_zh.md](docs/BUILD_zh.md)，英文版为 [docs/BUILD.md](docs/BUILD.md)。不要在本文件中重复维护完整编译步骤。

常用快速命令（Linux）：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j$(nproc)  # 编译
cd build && ctest --output-on-failure                                       # 全部测试
cd build && ctest -R test_state_manager --output-on-failure                 # 单个测试
```

## 项目目录结构

```
trafficlight4ai/
├── CMakeLists.txt                 # 根构建文件
├── CLAUDE.md                      # 项目指南
├── .github/workflows/             # CI（build.yml 编译验证 + release-packages.yml 发布打包）
├── docs/
│   ├── BUILD.md / BUILD_zh.md     # 跨平台构建指南（双语）
│   └── ...                        # 问题记录与设计说明
├── src/
│   ├── CMakeLists.txt             # tl4ai_core 静态库 + trafficlight4ai 可执行文件
│   ├── StateManager.h/cpp         # 状态机（纯逻辑，含超时机制）
│   ├── ConfigManager.h/cpp        # JSON 配置管理
│   ├── IpcServer.h/cpp            # QLocalServer 本地 IPC 服务端
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
│   └── tl4ai_ctl.cpp              # Qt QLocalSocket CLI
├── translations/
│   ├── trafficlight4ai_zh.ts      # 中文翻译
│   └── trafficlight4ai_ja.ts      # 日语翻译
├── packaging/
│   └── linux/                     # deb/rpm/AppImage/Arch 打包脚本 + .desktop
└── resources/
    ├── resources.qrc
    └── images/                    # 红绿灯 PNG + 应用图标
```

## 架构

### 两个可执行文件

- **trafficlight4ai**（`src/`）— Qt GUI 主进程，内嵌 IPC Server
- **tl4ai-ctl**（`tools/`）— Qt 轻量 CLI，通过 QLocalSocket 发送状态指令

### 核心类（`src/`）

| 类 | 职责 | 依赖 |
|---|------|------|
| `StateManager` | 状态机（Working/WaitingConfirm/Idle），超时自动回 Idle | Qt Core |
| `ConfigManager` | JSON 配置读写（`~/.config/trafficlight4ai/config.json`） | Qt Core |
| `IpcServer` | QLocalServer 监听本地 IPC socket，解析指令转发给 StateManager | Qt Network |
| `AiToolStrategy` | 策略接口，封装各 AI 工具的差异（hooks 模板、默认超时等） | 无 |
| `TrafficLightWidget` | 自定义 QWidget，绘制三灯 UI，支持呼吸灯/经典闪烁 | Qt Widgets |
| `FloatingWindow` | 无边框置顶窗口，可拖动，记忆位置 | Qt Widgets |
| `TrayIcon` | 系统托盘图标，颜色随状态变化，右键菜单 | Qt Widgets |
| `SettingsDialog` | 设置对话框，实时预览配置变更，取消可撤销，含查看/编辑 Hooks 配置 | Qt Widgets |
| `SoundUtils` | 音效播放（QMediaPlayer，支持 WAV/MP3/OGG，fallback 系统 beep） | Qt Multimedia |

### 数据流

```
AI Tool Hooks → tl4ai-ctl (Qt CLI) → local IPC socket → IpcServer → StateManager → TrafficLightWidget + TrayIcon
```

### 库分层

`tl4ai_core`（静态库）包含 StateManager、ConfigManager、IpcServer，供测试和主程序共用。GUI 组件仅在主程序中链接。

### Socket 协议

Linux 默认路径：`$XDG_RUNTIME_DIR/trafficlight4ai.sock`（fallback `/tmp/trafficlight4ai-$UID.sock`）。Windows 默认名称：`trafficlight4ai`。可通过 config.json 或 `TL4AI_SOCKET` 环境变量覆盖。

指令为单行纯文本：`RED\n` / `YELLOW\n` / `GREEN\n`，大小写不敏感，无法识别的指令静默忽略。

`tl4ai-ctl` 在 Linux 上启动时会非阻塞 drain stdin（AI 工具 hooks 会往 stdin 写 JSON 数据），Windows 上跳过。

### AI 工具策略模式

`AiToolStrategy` 接口封装不同 AI 工具的差异，通过 `AiToolRegistry` 注册和查找：
- `CodexStrategy` — Codex hooks 配置模板（`~/.codex/hooks.json`，整文件）
- `ClaudeCodeStrategy` — Claude Code hooks 配置模板（`~/.claude/settings.json`，仅 hooks 字段）
- `QoderCnStrategy` — Qoder CN hooks 配置模板（`~/.qoder-cn/settings.json`，仅 hooks 字段）
- `CopilotStrategy` — Copilot hooks 配置模板（`~/.copilot/hooks/trafficlight4ai.json`，整文件）

接口方法：`id()`、`displayName()`、`defaultTimeoutSec()`、`hooksTemplate()`、`hooksConfigPath()`、`hooksIsEntireFile()`。

新增工具只需添加一个 Strategy 实现并注册到 Registry。

## 测试

使用 Qt Test（QTest），每个测试为独立可执行文件：

| 测试 | 覆盖范围 |
|------|---------|
| `test_state_manager` | 状态切换、signal 发射、命令解析、超时机制 |
| `test_config_manager` | 默认值、读写持久化、容错回退、参数校验、aiTool/timeoutSec |
| `test_ipc_server` | socket 收发、无效指令、旧 socket 清理、restart |
| `test_ai_tool_strategy` | 事件名验证、hooks 模板内容、Registry 查找、hooksConfigPath/hooksIsEntireFile |
| `test_tl4ai_ctl` | CLI 集成测试，需要编译后的 tl4ai-ctl 二进制 |

### 注意事项

- 每个测试方法使用独立的临时文件路径（`init()` slot 生成唯一路径），避免测试间污染
- 构造 stale Unix socket 需用 POSIX `socket()/bind()/close()`，`QLocalServer::close()` 会自动 unlink
- IpcServer 测试中 `sendCommand` 后需 `QTest::qWait(100)` 等待事件循环处理

## 配置文件

路径：`~/.config/trafficlight4ai/config.json`

| 字段 | 默认值 | 说明 |
|------|--------|------|
| `language` | `"en"` | 界面语言（en/zh/ja） |
| `aiTool` | `"codex"` | AI 工具（codex/claude-code/qoder-cn/copilot） |
| `timeoutSec` | `300` | 超时回绿灯秒数，0 禁用 |
| `window.size` | `"medium"` | 窗口大小（xsmall/small/medium/large/xlarge） |
| `window.posX/posY` | `20` | 窗口位置 |
| `animation.mode` | `"breathing"` | 动画模式（breathing/classic） |
| `animation.periodMs` | `1000` | 动画周期 200~5000ms |
| `socket.path` | 平台相关 | Linux: `$XDG_RUNTIME_DIR/trafficlight4ai.sock`，Windows: `trafficlight4ai` |
| `sound.yellowEnabled/greenEnabled` | `true` | 提示音开关 |
| `sound.yellowFile/greenFile` | `""` | 自定义音效路径（WAV/MP3/OGG），空用系统 beep |

## 已知平台问题

- **GitHub Actions workflow 推送**：修改 `.github/workflows/` 需要 PAT 包含 `workflow` scope，否则推送被拒绝
- **openSUSE Leap 15.6 CI**：默认 GCC 7 不支持 Qt 6 所需的 C++17，需用 `gcc-13`/`g++-13`
- **Ubuntu SNI 托盘**：快速频繁 `setIcon` 后切换到静态图标可能不刷新，需 `QTimer::singleShot(150, ...)` 延迟重设
- **QSystemTrayIcon 不是 QWidget**：`QMenu` 不能用 `setParent(this)`，需用 `destroyed` signal 管理生命周期
- **FloatingWindow 尺寸切换**：需用 `QTimer::singleShot(0, ...)` 延迟 `move()` 以在布局重算后恢复位置
- **AppImage 构建 CDN 超时**：linuxdeploy/type2-runtime 下载可能因 GitHub CDN 504 失败，`build-appimage.sh` 已实现固定版本+continuous fallback 机制和 SHA256 校验
- **QLocalServer::listen() 与普通文件**：Qt 可能在路径被普通文件占用时仍允许 listen 成功，需在调用前用 `pathBlockedByNonSocket()` 显式拒绝

## 代码约定

- 类名 PascalCase，方法和变量 camelCase
- Qt 信号槽使用新式语法（`&Class::signal`）
- 头文件使用 `#pragma once`
- 头文件中使用的 Qt 类型必须显式 `#include`，不要依赖间接包含（CI 容器环境下隐式包含可能不存在）
- 私有成员变量前缀 `m_`
- 所有 UI 文本使用 `tr()` 包裹（国际化），同步更新 `translations/` 下的 `.ts` 文件
- 新增测试使用 `add_tl4ai_test(test_<component>)` 宏注册，测试槽函数按行为命名（如 `duplicateCommandRefreshesTimeout`）
- Git 提交使用 conventional-style 前缀：`feat:` / `fix:` / `docs:` / `debug:`，标题用祈使语气
- IpcServer 客户端读取需聚合至换行符或断开，不可在首次 `readyRead` 后立即处理（防止分片写入丢命令）
- 发布新 Release 后，同步更新 README.md、README_zh.md、docs/BUILD.md、docs/BUILD_zh.md 中的版本号和下载链接
- 更新 README.md 时必须同步更新 README_zh.md，保持双语内容一致
