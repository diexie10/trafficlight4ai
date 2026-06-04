# trafficlight4cc 设计文档

## 概述

trafficlight4cc 是一个 C++ Qt6 桌面应用，为 Claude Code 提供可视化红绿灯状态指示器。通过 Claude Code Hooks 机制自动感知工作状态，以红/黄/绿三色灯直观展示。

## 技术栈

- C++17、Qt 6、CMake
- 目标平台：Linux
- 状态检测：Claude Code Hooks + Unix Domain Socket

## 架构：单进程方案

```
Claude Code Hooks → tl4cc-ctl (CLI) → Unix Domain Socket → trafficlight4cc (Qt GUI + IPC Server)
```

- **trafficlight4cc**：Qt GUI 主进程，内嵌 IPC Server
- **tl4cc-ctl**：独立轻量 CLI，纯 POSIX 实现，不依赖 Qt

## 状态机

### LightState 枚举

| 状态 | 灯色 | 显示模式 | 含义 |
|------|------|---------|------|
| `Working` | 红 | 闪烁（默认呼吸灯） | Claude Code 正在执行 |
| `WaitingConfirm` | 黄 | 闪烁（默认呼吸灯） | 等待用户确认 |
| `Idle` | 绿 | 常亮 | 工作完成/空闲 |

### 转换规则

- 任意状态可直接跳转到任意其他状态（外部指令驱动）
- 启动默认为 `Idle`（绿灯常亮）
- `RED` → Working、`YELLOW` → WaitingConfirm、`GREEN` → Idle

### StateManager 类

- 持有当前 LightState
- `setState(LightState)` 方法
- 状态变化时发出 `stateChanged(LightState)` signal
- 重复设置相同状态不重复发 signal
- 纯逻辑类，不依赖 GUI，可独立测试

## GUI 层

### TrafficLightWidget（自定义 QWidget）

- 三个纵向排列的圆形灯（红/黄/绿），外围圆角矩形深灰底框
- 三档尺寸：Small（80x200）、Medium（100x260）、Large（130x340），默认 Small
- 当前激活灯显示对应颜色，非激活灯显示暗灰色
- 闪烁模式：
  - **呼吸灯**（默认）：QPropertyAnimation 驱动 alpha 值在 0.3~1.0 之间平滑过渡
  - **经典闪烁**：亮/灭交替
- 闪烁周期可配置，默认 1000ms
- Idle 状态绿灯常亮，不闪烁

### FloatingWindow（悬浮窗口）

- 无边框窗口（`Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint`）
- 半透明背景，圆角
- 鼠标左键拖动，松开后记忆位置到 config.json
- 默认位置：屏幕左上角（偏移 20px）

### TrayIcon（系统托盘）

- QSystemTrayIcon，图标为小圆点，颜色随状态变化
- 右键菜单：显示/隐藏悬浮窗、设置、退出
- 左键单击：切换悬浮窗显示/隐藏

## IPC 层

### IPC Server（主进程内嵌）

- QLocalServer 监听 Unix Domain Socket
- Socket 路径：`/tmp/trafficlight4cc.sock`（可配置）
- 启动时删除旧 socket 文件再创建，退出时清理
- 每个连接读取一行文本指令，处理后关闭连接
- 收到指令后调用 StateManager::setState()

### 协议

单行纯文本，换行符结尾：
- `RED\n` → Working
- `YELLOW\n` → WaitingConfirm
- `GREEN\n` → Idle
- 无法识别的指令静默忽略

### tl4cc-ctl（独立 CLI）

- 纯 C++/POSIX socket 实现，不依赖 Qt
- 用法：`tl4cc-ctl <red|yellow|green>`
- 逻辑：connect → send → close → exit(0)
- 连接失败时静默退出 exit(0)，无错误输出
- 超时 100ms

## 配置系统

### 配置文件

路径：`~/.config/trafficlight4cc/config.json`

```json
{
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
    "path": "/tmp/trafficlight4cc.sock"
  }
}
```

### 配置项

| 项 | 类型 | 可选值 | 默认值 |
|---|------|-------|-------|
| `window.size` | string | `"small"`, `"medium"`, `"large"` | `"small"` |
| `window.posX` | int | 像素 | `20` |
| `window.posY` | int | 像素 | `20` |
| `animation.mode` | string | `"breathing"`, `"classic"` | `"breathing"` |
| `animation.periodMs` | int | 200~5000 | `1000` |
| `socket.path` | string | 路径 | `"/tmp/trafficlight4cc.sock"` |

### ConfigManager 类

- 启动时加载配置，文件不存在则创建默认配置
- `get<T>(key)` / `set(key, value)` 接口
- 使用 QJsonDocument 解析
- 窗口拖动结束 / 切换尺寸时自动保存
- 配置文件损坏时回退默认值

## Claude Code Hooks 集成

### 默认推荐配置（~/.claude/settings.json）

```json
{
  "hooks": {
    "PreToolUse": [{ "command": "tl4cc-ctl red" }],
    "PostToolUse": [{ "command": "tl4cc-ctl red" }],
    "SubagentStart": [{ "command": "tl4cc-ctl red" }],
    "SubagentStop": [{ "command": "tl4cc-ctl red" }],
    "Notification": [{ "command": "tl4cc-ctl yellow" }],
    "PermissionRequest": [{ "command": "tl4cc-ctl yellow" }],
    "Stop": [{ "command": "tl4cc-ctl green" }],
    "SessionEnd": [{ "command": "tl4cc-ctl green" }]
  }
}
```

映射逻辑：
- **红灯**：工具调用前后、子代理活动 → 正在工作
- **黄灯**：通知、权限请求 → 需要用户注意/确认
- **绿灯**：停止、会话结束 → 完成

用户可在 config.json 中自定义 hook 事件到灯状态的映射。

## 测试策略

### 框架：Qt Test（QTest）

### 测试用例

**1. test_state_manager**
- 初始状态为 Idle
- 各指令正确切换状态
- 状态切换时 signal 正确发出
- 重复设置相同状态不重复发 signal

**2. test_config_manager**
- 文件不存在时创建默认配置
- 正确读取已有配置
- 修改后正确写入
- 格式损坏时回退默认值

**3. test_ipc_server**
- 发送 RED/YELLOW/GREEN 后状态正确
- 无效指令静默忽略
- 多客户端并发连接正确处理

**4. test_tl4cc_ctl**
- 正常发送指令后 exit(0)
- 主进程未运行时 exit(0) 无输出
- 超时机制生效

**5. TrafficLightWidget**
- 不做自动化渲染测试
- 通过 StateManager mock 验证响应 stateChanged signal

### CMake 测试结构

每个测试编译为独立可执行文件，通过 CTest 统一运行。
