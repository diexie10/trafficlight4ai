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
AI 工具 Hooks → tl4ai-ctl (CLI) → 本地 IPC socket → trafficlight4ai (GUI)
```

AI 工具的 hook 机制在合适的时机触发 `tl4ai-ctl red/yellow/green`，GUI 即时更新。

## 下载

预编译包可从 GitHub Releases 下载：

- `trafficlight4ai-v0.1.1-ubuntu-x86_64.tar.gz` — Ubuntu x86_64 构建。
- `trafficlight4ai-v0.1.1-windows-x86_64.zip` — Windows x86_64 构建，已通过 `windeployqt` 打包 Qt 运行库。
- `SHA256SUMS.txt` — 发布包校验和。

Windows 上运行 `bin/trafficlight4ai.exe`。它会作为 GUI 程序启动，不应弹出命令行窗口。可在 PowerShell、cmd 或 AI 工具 hooks 中调用 `bin/tl4ai-ctl.exe red/yellow/green` 来切换灯状态。如果 Windows 提示缺少 MSVC 运行时 DLL，请安装 Microsoft Visual C++ Redistributable 2022 x64。

## 从源码编译

Linux 和 Windows 的编译前提、编译命令、打包注意事项和验证方式见 [docs/BUILD_zh.md](docs/BUILD_zh.md)。

## 快速开始

### 1. 启动 GUI

```bash
./build/src/trafficlight4ai &
```

### 2. 让 hooks 能找到 tl4ai-ctl

Hooks 需要能找到 `tl4ai-ctl`，以下两种方式任选其一：

**方式 A** — 在 hook 命令中直接使用绝对路径（无需安装）：

在第 3 步的配置中，将 `tl4ai-ctl` 替换为完整路径，例如：

```
/home/you/trafficlight4ai/build/tools/tl4ai-ctl red
```

**方式 B** — 加入 PATH（这样 hooks 可以直接用短名 `tl4ai-ctl`）：

```bash
# 创建符号链接到 PATH 目录
sudo ln -s "$(pwd)/build/tools/tl4ai-ctl" /usr/local/bin/tl4ai-ctl

# 或复制到 ~/.local/bin（无需 sudo，确保 ~/.local/bin 已在 PATH 中）
cp build/tools/tl4ai-ctl ~/.local/bin/
```

### 3. 配置 AI 工具的 hooks

以下示例使用短名 `tl4ai-ctl`。如果你选择了方式 A，请替换为完整路径。

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
  - 窗口大小（超小 / 小 / 中 / 大 / 超大）
  - 动画模式（呼吸灯 / 经典闪烁）
  - 动画周期（200~5000ms）
  - Socket 路径
- **策略模式** — 轻松添加新 AI 工具（实现 `AiToolStrategy` 接口，注册到 `AiToolRegistry`）
- **提示音** — 黄灯（需确认）和绿灯（完成）时播放提示音，支持 WAV/MP3/OGG，fallback 系统 beep
- **国际化** — 英语（默认）、中文、日语，运行时切换
- **轻量 CLI** — `tl4ai-ctl` 使用 Qt Core/Network 和 `QLocalSocket` 实现跨平台本地 IPC

## 配置文件

路径：`~/.config/trafficlight4ai/config.json`

```json
{
  "language": "en",
  "aiTool": "codex",
  "timeoutSec": 300,
  "window": {
    "size": "medium",
    "posX": 20,
    "posY": 20
  },
  "animation": {
    "mode": "breathing",
    "periodMs": 1000
  },
  "socket": {
    "path": "$XDG_RUNTIME_DIR/trafficlight4ai.sock"
  },
  "sound": {
    "yellowEnabled": true,
    "greenEnabled": true,
    "yellowFile": "",
    "greenFile": ""
  }
}
```

Socket 路径或名称也可通过 `TL4AI_SOCKET` 环境变量覆盖。

## 许可证

MIT
