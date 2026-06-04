# trafficlight4ai

AI 编码工具的可视化红绿灯状态指示器（支持 Codex、Claude Code 等）。

[English](README.md)

## 功能简介

当 AI 编码助手在终端中运行时，trafficlight4ai 让你一眼看到当前状态：

- **红灯闪烁** — AI 正在工作（工具调用、子代理、思考中）
- **黄灯闪烁** — AI 需要你确认（权限请求、通知）
- **绿灯常亮** — 完成，等待你的下一个指令

展现形式：桌面上的悬浮小窗口（始终置顶）+ 系统托盘图标。

## 工作原理

```
AI 工具 Hooks → tl4ai-ctl (CLI) → Unix Domain Socket → trafficlight4ai (GUI)
```

AI 工具的 hook 机制在合适的时机触发 `tl4ai-ctl red/yellow/green`，GUI 即时更新。

## 环境要求

- Linux
- Qt 6（Core, Widgets, Network）
- CMake 3.20+
- C++17 编译器

## 编译

```bash
sudo apt install qt6-base-dev    # Ubuntu/Debian

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## 快速开始

### 1. 启动 GUI

```bash
./build/src/trafficlight4ai &
```

### 2. 将 tl4ai-ctl 加入 PATH

```bash
sudo ln -s $(pwd)/build/tools/tl4ai-ctl /usr/local/bin/tl4ai-ctl
```

### 3. 配置 AI 工具的 hooks

**Codex** — 创建 `~/.codex/hooks.json`：

```json
{
  "hooks": {
    "UserPromptSubmit": [
      { "hooks": [{ "type": "command", "command": "tl4ai-ctl red" }] }
    ],
    "PreToolUse": [
      { "hooks": [{ "type": "command", "command": "tl4ai-ctl red" }] }
    ],
    "SubagentStart": [
      { "hooks": [{ "type": "command", "command": "tl4ai-ctl red" }] }
    ],
    "PermissionRequest": [
      { "hooks": [{ "type": "command", "command": "tl4ai-ctl yellow" }] }
    ],
    "Stop": [
      { "hooks": [{ "type": "command", "command": "tl4ai-ctl green" }] }
    ]
  }
}
```

**Claude Code** — 添加到 `~/.claude/settings.json`：

```json
{
  "hooks": {
    "UserPromptSubmit": [{ "command": "tl4ai-ctl red" }],
    "PreToolUse": [{ "command": "tl4ai-ctl red" }],
    "SubagentStart": [{ "command": "tl4ai-ctl red" }],
    "Notification": [{ "command": "tl4ai-ctl yellow" }],
    "PermissionRequest": [{ "command": "tl4ai-ctl yellow" }],
    "Stop": [{ "command": "tl4ai-ctl green" }],
    "SessionEnd": [{ "command": "tl4ai-ctl green" }]
  }
}
```

也可以在设置对话框中点击"查看推荐 Hooks 配置"按钮获取模板。

### 4. 手动测试

```bash
tl4ai-ctl red      # 红灯闪烁
tl4ai-ctl yellow   # 黄灯闪烁
tl4ai-ctl green    # 绿灯常亮
```

## 功能特性

- **悬浮窗口** — 可拖动、始终置顶、记忆位置
- **系统托盘图标** — 颜色随状态变化，右键菜单
- **设置对话框** — 实时预览，取消可撤销
  - AI 工具选择（Codex / Claude Code）
  - 超时自动回绿灯（默认 5 分钟，0 禁用）
  - 窗口大小（小 / 中 / 大）
  - 动画模式（呼吸灯 / 经典闪烁）
  - 动画周期（200~5000ms）
  - Socket 路径
- **策略模式** — 轻松添加新 AI 工具（实现 `AiToolStrategy` 接口，注册到 `AiToolRegistry`）
- **轻量 CLI** — `tl4ai-ctl` 纯 POSIX C++ 实现，不依赖 Qt，执行 <100ms

## 配置文件

路径：`~/.config/trafficlight4ai/config.json`

```json
{
  "aiTool": "codex",
  "timeoutSec": 300,
  "window": {
    "size": "small",
    "posX": 20,
    "posY": 20
  },
  "animation": {
    "mode": "breathing",
    "periodMs": 1000
  },
  "socket": {
    "path": "/tmp/trafficlight4ai.sock"
  }
}
```

Socket 路径也可通过 `TL4AI_SOCKET` 环境变量覆盖。

## 测试

```bash
cd build && ctest --output-on-failure
```

## 许可证

MIT
