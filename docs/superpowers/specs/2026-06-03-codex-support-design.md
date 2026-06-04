# Codex 支持 + 项目重命名设计文档

## 概述

为 trafficlight4ai（原 trafficlight4cc）添加 Codex 支持，采用策略模式封装不同 AI 工具的差异。新增超时自动回绿灯机制。全局重命名 4cc → 4ai。

## 全局重命名

| 旧 | 新 |
|---|---|
| `trafficlight4cc` | `trafficlight4ai` |
| `tl4cc-ctl` | `tl4ai-ctl` |
| `tl4cc_core` | `tl4ai_core` |
| `TL4CC_SOCKET` | `TL4AI_SOCKET` |
| `/tmp/trafficlight4cc.sock` | `/tmp/trafficlight4ai.sock` |
| `~/.config/trafficlight4cc/` | `~/.config/trafficlight4ai/` |

## AiToolStrategy 策略模式

### 接口

```cpp
class AiToolStrategy {
public:
    virtual ~AiToolStrategy() = default;
    virtual QString id() const = 0;           // "codex", "claude-code"
    virtual QString displayName() const = 0;  // "Codex", "Claude Code"
    virtual int defaultTimeoutSec() const = 0;
    virtual QString hooksTemplate() const = 0;
};
```

### 实现

**CodexStrategy**
- id: `"codex"`，displayName: `"Codex"`
- defaultTimeoutSec: 300
- hooksTemplate: `~/.codex/hooks.json` 格式，调用 `tl4ai-ctl`

**ClaudeCodeStrategy**
- id: `"claude-code"`，displayName: `"Claude Code"`
- defaultTimeoutSec: 300
- hooksTemplate: `~/.claude/settings.json` 格式，调用 `tl4ai-ctl`

### AiToolRegistry

静态工具类：
- `strategies()` — 返回所有已注册 strategy 列表
- `find(const QString &id)` — 按 id 查找

新增工具时只需添加一个 Strategy 实现并注册。

### 文件

`src/AiToolStrategy.h` — 接口 + Registry + 所有实现（代码量小，放一个文件）

## 超时机制

### StateManager 新增

- `QTimer m_timeoutTimer`
- Working / WaitingConfirm 时启动定时器
- Idle 时停止定时器
- 定时器到期自动 setState(LightState::Idle)
- `setTimeoutSec(int sec)` — 0 表示禁用
- `timeoutSec() const`

### ConfigManager 新增

- `"aiTool": "codex"` — AI 工具 id，默认 codex
- `"timeoutSec": 300` — 超时秒数，0 禁用，范围 0 或 30~3600
- `aiTool()` / `setAiTool()`
- `timeoutSec()` / `setTimeoutSec()`

## 设置对话框变更

### 新增控件

| 配置项 | 控件 | 行为 |
|--------|------|------|
| AI 工具 | QComboBox（从 Registry 动态填充） | 切换后更新 tooltip、窗口标题 |
| 超时时间 | QSpinBox（0~3600 秒，0=禁用） | 实时生效，更新 StateManager |

### AI 工具切换联动

- 更新 ConfigManager 的 aiTool
- 更新 TrayIcon tooltip（"Traffic Light for Codex" / "Traffic Light for Claude Code"）
- 不自动改超时值

### Hooks 配置模板

设置对话框底部"查看推荐 Hooks 配置"按钮，点击弹出只读 QTextEdit + "复制"按钮，显示当前所选工具的 hooks 模板。

### 快照撤销

新增 aiTool 和 timeoutSec 到快照中。

## TrayIcon 动态化

tooltip 根据当前 aiTool 的 displayName 动态显示。新增 `setToolTipText(const QString &)` 或直接在 aiTool 切换时调用 `setToolTip()`。

## 测试策略

### test_state_manager 新增

- Working 超时自动回 Idle
- WaitingConfirm 超时自动回 Idle
- Idle 不触发超时
- 超时 0 禁用
- 状态切换重置定时器

### test_config_manager 新增

- aiTool 默认 "codex"
- setAiTool / aiTool 读写
- timeoutSec 默认 300
- setTimeoutSec 范围限制（0 或 30~3600）
- 持久化正确

### 不需要自动化测试

- SettingsDialog UI
- Hooks 模板文本
- AiToolStrategy（纯数据类）
- 全局重命名（编译通过即验证）
