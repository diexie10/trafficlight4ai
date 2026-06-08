#pragma once

#include <QString>
#include <QDir>
#include <QList>
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

class AiToolRegistry {
public:
    static const QList<AiToolStrategy *> &strategies()
    {
        static CodexStrategy codex;
        static ClaudeCodeStrategy claude;
        static QoderCnStrategy qoderCn;
        static QList<AiToolStrategy *> list = {&codex, &claude, &qoderCn};
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
};
