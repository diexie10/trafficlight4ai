#pragma once

#include <QString>
#include <QDir>
#include <QList>
#include <QFile>
#include <QCoreApplication>
#include <memory>

class AiToolStrategy {
public:
    virtual ~AiToolStrategy() = default;
    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    virtual int defaultTimeoutSec() const = 0;
    virtual QString hooksTemplate() const = 0;
    virtual QString hooksConfigPath() const = 0;
    virtual bool hooksIsEntireFile() const = 0;
};

class CodexStrategy : public AiToolStrategy {
public:
    QString id() const override { return "codex"; }
    QString displayName() const override { return "Codex"; }
    int defaultTimeoutSec() const override { return 300; }
    QString hooksTemplate() const override
    {
        return R"({
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
})";
    }
    QString hooksConfigPath() const override { return QDir::homePath() + "/.codex/hooks.json"; }
    bool hooksIsEntireFile() const override { return true; }
};

class ClaudeCodeStrategy : public AiToolStrategy {
public:
    QString id() const override { return "claude-code"; }
    QString displayName() const override { return "Claude Code"; }
    int defaultTimeoutSec() const override { return 300; }
    QString hooksTemplate() const override
    {
        return R"({
  "hooks": {
    "UserPromptSubmit": [{ "command": "tl4ai-ctl red" }],
    "PreToolUse": [{ "command": "tl4ai-ctl red" }],
    "SubagentStart": [{ "command": "tl4ai-ctl red" }],
    "Notification": [{ "command": "tl4ai-ctl yellow" }],
    "PermissionRequest": [{ "command": "tl4ai-ctl yellow" }],
    "Stop": [{ "command": "tl4ai-ctl green" }],
    "SessionEnd": [{ "command": "tl4ai-ctl green" }]
  }
})";
    }
    QString hooksConfigPath() const override { return QDir::homePath() + "/.claude/settings.json"; }
    bool hooksIsEntireFile() const override { return false; }
};

class QoderCnStrategy : public AiToolStrategy {
public:
    QString id() const override { return "qoder-cn"; }
    QString displayName() const override { return "Qoder CN"; }
    int defaultTimeoutSec() const override { return 300; }
    QString hooksTemplate() const override
    {
        return R"({
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
})";
    }
    QString hooksConfigPath() const override { return QDir::homePath() + "/.qoder-cn/settings.json"; }
    bool hooksIsEntireFile() const override { return false; }
};

class CopilotStrategy : public AiToolStrategy {
public:
    QString id() const override { return "copilot"; }
    QString displayName() const override { return "Copilot"; }
    int defaultTimeoutSec() const override { return 300; }
    QString hooksTemplate() const override
    {
        return R"({
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
})";
    }
    QString hooksConfigPath() const override { return QDir::homePath() + "/.copilot/hooks/trafficlight4ai.json"; }
    bool hooksIsEntireFile() const override { return true; }
};

class GeminiStrategy : public AiToolStrategy {
public:
    QString id() const override { return "gemini"; }
    QString displayName() const override { return "Gemini"; }
    int defaultTimeoutSec() const override { return 300; }
    QString hooksTemplate() const override
    {
        return R"({
  "hooks": {
    "BeforeAgent": [
      {
        "hooks": [{ "type": "command", "command": "tl4ai-ctl red" }]
      }
    ],
    "BeforeTool": [
      {
        "hooks": [{ "type": "command", "command": "tl4ai-ctl red" }]
      }
    ],
    "Notification": [
      {
        "hooks": [{ "type": "command", "command": "tl4ai-ctl yellow" }]
      }
    ],
    "AfterAgent": [
      {
        "hooks": [{ "type": "command", "command": "tl4ai-ctl green" }]
      }
    ],
    "SessionEnd": [
      {
        "hooks": [{ "type": "command", "command": "tl4ai-ctl green" }]
      }
    ]
  }
})";
    }
    QString hooksConfigPath() const override { return QDir::homePath() + "/.gemini/settings.json"; }
    bool hooksIsEntireFile() const override { return false; }
};

class OpenCodeStrategy : public AiToolStrategy {
public:
    QString id() const override { return "opencode"; }
    QString displayName() const override { return "OpenCode"; }
    int defaultTimeoutSec() const override { return 300; }
    QString hooksTemplate() const override
    {
        return R"(/**
 * trafficlight4ai — OpenCode 插件
 * 根据 OpenCode 状态自动切换桌面红绿灯：
 *   🟢 空闲/完成
 *   🟡 AI 思考中/等待确认
 *   🔴 执行工具（读写文件/跑命令）
 */
const CTL = require("child_process");
// tl4ai-ctl 会被 Settings 对话框自动替换为完整路径
const CTL_PATH = "tl4ai-ctl";

function setLight(color) {
  try {
    CTL.execFileSync(CTL_PATH, [color], { timeout: 3000, windowsHide: true });
  } catch (_) {}
}

module.exports = {
  TrafficLightPlugin: async () => {
    setLight("green");
    return {
      "session.created": async () => { setLight("green"); },
      "session.idle": async () => { setLight("green"); },
      "session.error": async () => { setLight("red"); },
      "session.deleted": async () => { setLight("green"); },
      "message.updated": async () => { setLight("yellow"); },
      "tool.execute.before": async () => { setLight("red"); },
      "tool.execute.after": async () => { setLight("yellow"); },
      "permission.asked": async () => { setLight("yellow"); },
      "permission.replied": async () => { setLight("green"); },
    };
  },
};)";
    }
    QString hooksConfigPath() const override { return QDir::homePath() + "/.config/opencode/plugins/trafficlight4ai.js"; }
    bool hooksIsEntireFile() const override { return true; }
};

class AiToolRegistry {
public:
    static const QList<AiToolStrategy *> &strategies()
    {
        static CodexStrategy codex;
        static ClaudeCodeStrategy claude;
        static QoderCnStrategy qoderCn;
        static CopilotStrategy copilot;
        static GeminiStrategy gemini;
        static OpenCodeStrategy opencode;
        static QList<AiToolStrategy *> list = {&codex, &claude, &qoderCn, &copilot, &gemini, &opencode};
        return list;
    }

    static AiToolStrategy *find(const QString &id)
    {
        for (auto *s : strategies()) {
            if (s->id() == id)
                return s;
        }
        return nullptr;
    }

    static QString resolvedCtlPath()
    {
        const QString dir = QCoreApplication::applicationDirPath();
#ifdef Q_OS_WIN
        const QString path = dir + "/tl4ai-ctl.exe";
#else
        const QString path = dir + "/tl4ai-ctl";
#endif
        if (!QFile::exists(path))
            return QString("tl4ai-ctl");
        return path.contains(' ') ? ('"' + path + '"') : path;
    }

    static QString resolvedTemplate(const AiToolStrategy *strategy)
    {
        return strategy->hooksTemplate().replace(
            QLatin1String("tl4ai-ctl"), resolvedCtlPath());
    }
};
