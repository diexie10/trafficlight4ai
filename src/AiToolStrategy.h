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
 * trafficlight4ai — OpenCode Plugin
 *
 * Auto-switches desktop traffic light based on AI agent state:
 *   🟢 Idle / Done
 *   🟡 AI thinking / Waiting for confirmation
 *   🔴 Executing tools
 *
 * This template requires OpenCode to be restarted after installation.
 * If tl4ai-ctl is not in PATH, set TL4AI_CTL_PATH env var.
 */

import { execFileSync, spawn } from "child_process";
import { existsSync } from "fs";
import { join, dirname } from "path";

const CTL_PATH = process.env.TL4AI_CTL_PATH || "tl4ai-ctl";

function setLight(color) {
  try {
    execFileSync(CTL_PATH, [color], { timeout: 3000, windowsHide: true });
  } catch (_) {}
}

// ── Auto-start GUI (derived from CTL_PATH) ──
(function ensureGui() {
  const dir = dirname(CTL_PATH);
  if (!dir || dir === ".") return;
  // tools/tl4ai-ctl(.exe) → src/trafficlight4ai(.exe)
  const gui = join(dir.replace(/[/\\]tools[/\\]?/, "\\src\\"), "trafficlight4ai");
  const exe = existsSync(gui) ? gui : gui + ".exe";
  if (!existsSync(exe)) return;
  const child = spawn(exe, [], { detached: true, stdio: "ignore", windowsHide: true });
  child.unref();
})();

const TrafficLightPlugin = async () => {
  setLight("green");

  return {
    "tool.execute.before": async () => { setLight("red"); },
    "tool.execute.after":  async () => { setLight("yellow"); },
    "permission.ask":      async () => { setLight("yellow"); },

    event: async ({ event }) => {
      switch (event.type) {
        case "session.status":
          if (event.properties?.status?.type === "idle")
            setLight("green");
          else if (event.properties?.status?.type === "busy" ||
                   event.properties?.status?.type === "retry")
            setLight("yellow");
          break;
        case "session.idle":    setLight("green"); break;
        case "session.error":   setLight("red");   break;
        case "session.created": setLight("green"); break;
        case "session.deleted": setLight("green"); break;
        case "message.part.updated": setLight("yellow"); break;
        case "message.updated":      setLight("yellow"); break;
        case "permission.replied": setLight("green");  break;
        case "permission.updated": setLight("yellow"); break;
      }
    },
  };
};

export { TrafficLightPlugin };)";
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
        // 不加外层引号，resolvedTemplate 会处理 JSON 转义
        return path;
    }

    static QString resolvedTemplate(const AiToolStrategy *strategy)
    {
        const QString rawPath = resolvedCtlPath();
        // JSON 中嵌入含空格的命令路径 → 用 \" 转义，否则外层 JSON 引号会冲突
        const QString escaped = rawPath.contains(' ')
            ? QStringLiteral("\\\"") + rawPath + QStringLiteral("\\\"")
            : rawPath;
        return strategy->hooksTemplate().replace(
            QLatin1String("tl4ai-ctl"), escaped);
    }
};
