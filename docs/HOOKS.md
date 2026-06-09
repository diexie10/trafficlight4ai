# Configure Hooks for Your AI Tool

The examples below use the short name `tl4ai-ctl`. If you installed trafficlight4ai to a custom location, replace it with the full path.

You can also view these templates in the Settings dialog ("View Recommended Hooks Config" button) or edit them directly ("Edit Hooks Config" button).

## Codex

Create `~/.codex/hooks.json`:

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

Add to `~/.claude/settings.json`:

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

Add to `~/.qoder-cn/settings.json` (or project-level `.qoder-cn/settings.json`):

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

Save as `~/.copilot/hooks/trafficlight4ai.json`:

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

## Configuration

Config file: `~/.config/trafficlight4ai/config.json`

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

The default socket path is platform-specific: Linux uses `$XDG_RUNTIME_DIR/trafficlight4ai.sock` (fallback `/tmp/trafficlight4ai-$UID.sock`), macOS uses `$TMPDIR/trafficlight4ai.sock`, and Windows uses the named pipe `trafficlight4ai`. It can also be overridden via `TL4AI_SOCKET` environment variable.
