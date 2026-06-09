# 配置 AI 工具的 Hooks

以下示例使用短名 `tl4ai-ctl`。如果你将 trafficlight4ai 安装到了自定义位置，请替换为完整路径。

也可以在设置对话框中点击"查看推荐 Hooks 配置"按钮获取模板，或点击"编辑 Hooks 配置"按钮直接编辑。

## Codex

创建 `~/.codex/hooks.json`：

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

## Claude Code

添加到 `~/.claude/settings.json`：

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

## Qoder CN

添加到 `~/.qoder-cn/settings.json`（或项目级 `.qoder-cn/settings.json`）：

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
    "Notification": [
      { "hooks": [{ "type": "command", "command": "tl4ai-ctl yellow" }] }
    ],
    "PermissionRequest": [
      { "hooks": [{ "type": "command", "command": "tl4ai-ctl yellow" }] }
    ],
    "Stop": [
      { "hooks": [{ "type": "command", "command": "tl4ai-ctl green" }] }
    ],
    "SessionEnd": [
      { "hooks": [{ "type": "command", "command": "tl4ai-ctl green" }] }
    ]
  }
}
```

## Copilot

保存为 `~/.copilot/hooks/trafficlight4ai.json`：

```json
{
  "version": 1,
  "hooks": {
    "userPromptSubmitted": [
      { "type": "command", "command": "tl4ai-ctl red" }
    ],
    "preToolUse": [
      { "type": "command", "command": "tl4ai-ctl red" }
    ],
    "subagentStart": [
      { "type": "command", "command": "tl4ai-ctl red" }
    ],
    "notification": [
      { "type": "command", "command": "tl4ai-ctl yellow" }
    ],
    "permissionRequest": [
      { "type": "command", "command": "tl4ai-ctl yellow" }
    ],
    "agentStop": [
      { "type": "command", "command": "tl4ai-ctl green" }
    ],
    "sessionEnd": [
      { "type": "command", "command": "tl4ai-ctl green" }
    ]
  }
}
```

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

默认 socket 路径因平台而异：Linux 使用 `$XDG_RUNTIME_DIR/trafficlight4ai.sock`（fallback `/tmp/trafficlight4ai-$UID.sock`），macOS 使用 `$TMPDIR/trafficlight4ai.sock`，Windows 使用命名管道 `trafficlight4ai`。也可通过 `TL4AI_SOCKET` 环境变量覆盖。
