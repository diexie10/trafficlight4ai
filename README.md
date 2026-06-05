# trafficlight4ai

A visual traffic light status indicator for AI coding tools (Codex, Claude Code, and more).

[中文文档](README_zh.md)

## What It Does

When your AI coding assistant is running in the terminal, trafficlight4ai shows you what's happening at a glance:

- **Red (blinking)** — AI is working (tool calls, subagents, thinking)
- **Yellow (blinking)** — AI needs your confirmation (permission requests, notifications)
- **Green (steady)** — Done, waiting for your next prompt

It displays as a small floating always-on-top window + system tray icon on your desktop.

## How It Works

```
AI Tool Hooks → tl4ai-ctl (CLI) → Unix Domain Socket → trafficlight4ai (GUI)
```

Your AI tool's hook system triggers `tl4ai-ctl red/yellow/green` at the right moments. The GUI updates instantly.

## Requirements

- Linux
- Qt 6 (Core, Widgets, Network, Multimedia)
- CMake 3.20+
- C++17 compiler

## Build

```bash
# qt6-base-dev: provides Core, Widgets, Network, Test modules (GUI, IPC, testing)
# qt6-multimedia-dev: provides Multimedia module (sound notification playback)
# qt6-tools-dev: provides LinguistTools module (i18n translation compilation)
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-tools-dev    # Ubuntu/Debian

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Quick Start

### 1. Run the GUI

```bash
./build/src/trafficlight4ai &
```

### 2. Add tl4ai-ctl to your PATH

```bash
sudo ln -s $(pwd)/build/tools/tl4ai-ctl /usr/local/bin/tl4ai-ctl
```

### 3. Configure hooks for your AI tool

**Codex** — Create `~/.codex/hooks.json`:

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

**Claude Code** — Add to `~/.claude/settings.json`:

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

You can also view these templates in the Settings dialog ("View Recommended Hooks Config" button).

### 4. Test manually

```bash
tl4ai-ctl red      # Red light blinks
tl4ai-ctl yellow   # Yellow light blinks
tl4ai-ctl green    # Green light steady
```

## Features

- **Floating window** — Draggable, always-on-top, remembers position
- **System tray icon** — Color changes with state, right-click menu
- **Settings dialog** — Live preview, cancel to undo
  - AI tool selection (Codex / Claude Code)
  - Timeout auto-idle (default 5 min, 0 to disable)
  - Window size (extra small / small / medium / large / extra large)
  - Animation mode (breathing / classic blink)
  - Animation period (200–5000ms)
  - Socket path
- **Strategy pattern** — Easy to add new AI tools (implement `AiToolStrategy`, register in `AiToolRegistry`)
- **Lightweight CLI** — `tl4ai-ctl` is pure POSIX C++, no Qt dependency, <100ms execution

## Configuration

Config file: `~/.config/trafficlight4ai/config.json`

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
    "path": "$XDG_RUNTIME_DIR/trafficlight4ai.sock"
  }
}
```

Socket path can also be overridden via `TL4AI_SOCKET` environment variable.

## Tests

```bash
cd build && ctest --output-on-failure
```

## License

MIT
