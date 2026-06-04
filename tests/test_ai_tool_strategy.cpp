#include <QtTest>
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

    void registryFallbackForUnknown()
    {
        auto *s = AiToolRegistry::find("unknown-tool");
        QVERIFY(s != nullptr); // fallback to first
        QCOMPARE(s->id(), QString("codex"));
    }

    void registryStrategiesNotEmpty()
    {
        QVERIFY(!AiToolRegistry::strategies().isEmpty());
    }
};

QTEST_MAIN(TestAiToolStrategy)
#include "test_ai_tool_strategy.moc"
