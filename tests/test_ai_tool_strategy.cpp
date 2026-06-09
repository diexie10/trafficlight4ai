#include <QtTest>
#include <QRegularExpression>
#include "AiToolStrategy.h"

class TestAiToolStrategy : public QObject {
    Q_OBJECT

private:
    // Valid Codex events per official docs
    const QStringList codexEvents = {
        "SessionStart", "UserPromptSubmit", "PreToolUse", "PermissionRequest",
        "PostToolUse", "PreCompact", "PostCompact", "SubagentStart",
        "SubagentStop", "Stop"
    };

private slots:
    void codexTemplateContainsUserPromptSubmit()
    {
        CodexStrategy codex;
        QVERIFY(codex.hooksTemplate().contains("UserPromptSubmit"));
    }

    void codexTemplateDoesNotContainTaskStarted()
    {
        CodexStrategy codex;
        QVERIFY(!codex.hooksTemplate().contains("TaskStarted"));
    }

    void codexTemplateOnlyUsesValidEvents()
    {
        CodexStrategy codex;
        const QString tmpl = codex.hooksTemplate();
        // Extract event names from template (keys in "hooks" object)
        QRegularExpression re(R"RE("(\w+)":\s*\[)RE");
        auto it = re.globalMatch(tmpl);
        while (it.hasNext()) {
            auto match = it.next();
            QString event = match.captured(1);
            if (event == "hooks")
                continue;
            QVERIFY2(codexEvents.contains(event),
                      qPrintable("Invalid Codex event: " + event));
        }
    }

    void codexTemplateHasRedYellowGreen()
    {
        CodexStrategy codex;
        const QString tmpl = codex.hooksTemplate();
        QVERIFY(tmpl.contains("tl4ai-ctl red"));
        QVERIFY(tmpl.contains("tl4ai-ctl yellow"));
        QVERIFY(tmpl.contains("tl4ai-ctl green"));
    }

    void claudeTemplateContainsUserPromptSubmit()
    {
        ClaudeCodeStrategy claude;
        QVERIFY(claude.hooksTemplate().contains("UserPromptSubmit"));
    }

    void claudeTemplateHasRedYellowGreen()
    {
        ClaudeCodeStrategy claude;
        const QString tmpl = claude.hooksTemplate();
        QVERIFY(tmpl.contains("tl4ai-ctl red"));
        QVERIFY(tmpl.contains("tl4ai-ctl yellow"));
        QVERIFY(tmpl.contains("tl4ai-ctl green"));
    }

    void claudeTemplateOnlyUsesValidEvents()
    {
        const QStringList claudeEvents = {
            "PreToolUse", "PostToolUse", "Notification", "Stop",
            "SubagentStart", "SubagentStop", "UserPromptSubmit",
            "PermissionRequest", "SessionEnd"
        };

        ClaudeCodeStrategy claude;
        const QString tmpl = claude.hooksTemplate();
        QRegularExpression re(R"RE("(\w+)":\s*[\[{])RE");
        auto it = re.globalMatch(tmpl);
        while (it.hasNext()) {
            auto match = it.next();
            QString event = match.captured(1);
            if (event == "hooks" || event == "command")
                continue;
            QVERIFY2(claudeEvents.contains(event),
                      qPrintable("Invalid Claude Code event: " + event));
        }
    }

    void qoderCnTemplateContainsUserPromptSubmit()
    {
        QoderCnStrategy qoderCn;
        QVERIFY(qoderCn.hooksTemplate().contains("UserPromptSubmit"));
    }

    void qoderCnTemplateHasRedYellowGreen()
    {
        QoderCnStrategy qoderCn;
        const QString tmpl = qoderCn.hooksTemplate();
        QVERIFY(tmpl.contains("tl4ai-ctl red"));
        QVERIFY(tmpl.contains("tl4ai-ctl yellow"));
        QVERIFY(tmpl.contains("tl4ai-ctl green"));
    }

    void qoderCnTemplateOnlyUsesValidEvents()
    {
        const QStringList qoderCnEvents = {
            "SessionStart", "SessionEnd", "UserPromptSubmit",
            "PreToolUse", "PostToolUse", "PostToolUseFailure",
            "Stop", "SubagentStart", "SubagentStop",
            "PreCompact", "Notification", "PermissionRequest"
        };

        QoderCnStrategy qoderCn;
        const QString tmpl = qoderCn.hooksTemplate();
        QRegularExpression re(R"RE("(\w+)":\s*\[)RE");
        auto it = re.globalMatch(tmpl);
        while (it.hasNext()) {
            auto match = it.next();
            QString event = match.captured(1);
            if (event == "hooks")
                continue;
            QVERIFY2(qoderCnEvents.contains(event),
                      qPrintable("Invalid Qoder CN event: " + event));
        }
    }

    void copilotTemplateContainsUserPromptSubmitted()
    {
        CopilotStrategy copilot;
        QVERIFY(copilot.hooksTemplate().contains("userPromptSubmitted"));
    }

    void copilotTemplateHasRedYellowGreen()
    {
        CopilotStrategy copilot;
        const QString tmpl = copilot.hooksTemplate();
        QVERIFY(tmpl.contains("tl4ai-ctl red"));
        QVERIFY(tmpl.contains("tl4ai-ctl yellow"));
        QVERIFY(tmpl.contains("tl4ai-ctl green"));
    }

    void copilotTemplateOnlyUsesValidEvents()
    {
        const QStringList copilotEvents = {
            "sessionStart", "sessionEnd", "userPromptSubmitted",
            "preToolUse", "postToolUse", "postToolUseFailure",
            "preCompact", "agentStop", "subagentStart",
            "subagentStop", "errorOccurred", "notification",
            "permissionRequest"
        };

        CopilotStrategy copilot;
        const QString tmpl = copilot.hooksTemplate();
        QRegularExpression re(R"RE("(\w+)":\s*\[)RE");
        auto it = re.globalMatch(tmpl);
        while (it.hasNext()) {
            auto match = it.next();
            QString event = match.captured(1);
            if (event == "hooks" || event == "version")
                continue;
            QVERIFY2(copilotEvents.contains(event),
                      qPrintable("Invalid Copilot event: " + event));
        }
    }

    void copilotTemplateHasVersion()
    {
        CopilotStrategy copilot;
        QVERIFY(copilot.hooksTemplate().contains("\"version\": 1"));
    }

    void codexHooksConfigPath()
    {
        CodexStrategy codex;
        QVERIFY(codex.hooksConfigPath().endsWith("/.codex/hooks.json"));
    }

    void codexHooksIsEntireFile()
    {
        CodexStrategy codex;
        QCOMPARE(codex.hooksIsEntireFile(), true);
    }

    void claudeHooksConfigPath()
    {
        ClaudeCodeStrategy claude;
        QVERIFY(claude.hooksConfigPath().endsWith("/.claude/settings.json"));
    }

    void claudeHooksIsNotEntireFile()
    {
        ClaudeCodeStrategy claude;
        QCOMPARE(claude.hooksIsEntireFile(), false);
    }

    void qoderCnHooksConfigPath()
    {
        QoderCnStrategy qoderCn;
        QVERIFY(qoderCn.hooksConfigPath().endsWith("/.qoder-cn/settings.json"));
    }

    void qoderCnHooksIsNotEntireFile()
    {
        QoderCnStrategy qoderCn;
        QCOMPARE(qoderCn.hooksIsEntireFile(), false);
    }

    void copilotHooksConfigPath()
    {
        CopilotStrategy copilot;
        QVERIFY(copilot.hooksConfigPath().endsWith("/.copilot/hooks/trafficlight4ai.json"));
    }

    void copilotHooksIsEntireFile()
    {
        CopilotStrategy copilot;
        QCOMPARE(copilot.hooksIsEntireFile(), true);
    }

    void codexDisplayName()
    {
        CodexStrategy codex;
        QCOMPARE(codex.displayName(), QString("Codex"));
    }

    void codexDefaultTimeout()
    {
        CodexStrategy codex;
        QCOMPARE(codex.defaultTimeoutSec(), 300);
    }

    void claudeDisplayName()
    {
        ClaudeCodeStrategy claude;
        QCOMPARE(claude.displayName(), QString("Claude Code"));
    }

    void claudeDefaultTimeout()
    {
        ClaudeCodeStrategy claude;
        QCOMPARE(claude.defaultTimeoutSec(), 300);
    }

    void qoderCnDisplayName()
    {
        QoderCnStrategy qoderCn;
        QCOMPARE(qoderCn.displayName(), QString("Qoder CN"));
    }

    void qoderCnDefaultTimeout()
    {
        QoderCnStrategy qoderCn;
        QCOMPARE(qoderCn.defaultTimeoutSec(), 300);
    }

    void copilotDisplayName()
    {
        CopilotStrategy copilot;
        QCOMPARE(copilot.displayName(), QString("Copilot"));
    }

    void copilotDefaultTimeout()
    {
        CopilotStrategy copilot;
        QCOMPARE(copilot.defaultTimeoutSec(), 300);
    }

    void registryFindsQoderCn()
    {
        auto *s = AiToolRegistry::find("qoder-cn");
        QVERIFY(s != nullptr);
        QCOMPARE(s->id(), QString("qoder-cn"));
    }

    void registryFindsCopilot()
    {
        auto *s = AiToolRegistry::find("copilot");
        QVERIFY(s != nullptr);
        QCOMPARE(s->id(), QString("copilot"));
    }

    void registryFindsCodex()
    {
        auto *s = AiToolRegistry::find("codex");
        QVERIFY(s != nullptr);
        QCOMPARE(s->id(), QString("codex"));
    }

    void registryFindsClaudeCode()
    {
        auto *s = AiToolRegistry::find("claude-code");
        QVERIFY(s != nullptr);
        QCOMPARE(s->id(), QString("claude-code"));
    }

    void registryReturnsNullForUnknown()
    {
        auto *s = AiToolRegistry::find("unknown-tool");
        QVERIFY(s == nullptr);
    }

    void registryStrategiesNotEmpty()
    {
        QVERIFY(!AiToolRegistry::strategies().isEmpty());
    }
};

QTEST_MAIN(TestAiToolStrategy)
#include "test_ai_tool_strategy.moc"
