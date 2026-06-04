#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include "ConfigManager.h"

class TestConfigManager : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    int m_testIndex = 0;
    QString m_configPath;

private slots:
    void init()
    {
        m_configPath = m_tempDir.path() + "/config_" + QString::number(m_testIndex++) + ".json";
    }
    void createsDefaultConfigWhenMissing()
    {
        ConfigManager cm(m_configPath);
        QVERIFY(QFile::exists(m_configPath));
    }

    void defaultWindowSize()
    {
        ConfigManager cm(m_configPath);
        QCOMPARE(cm.windowSize(), QString("small"));
    }

    void defaultWindowPosition()
    {
        ConfigManager cm(m_configPath);
        QCOMPARE(cm.windowPosX(), 20);
        QCOMPARE(cm.windowPosY(), 20);
    }

    void defaultAnimationMode()
    {
        ConfigManager cm(m_configPath);
        QCOMPARE(cm.animationMode(), QString("breathing"));
    }

    void defaultAnimationPeriod()
    {
        ConfigManager cm(m_configPath);
        QCOMPARE(cm.animationPeriodMs(), 1000);
    }

    void defaultSocketPath()
    {
        ConfigManager cm(m_configPath);
        QCOMPARE(cm.socketPath(), QString("/tmp/trafficlight4ai.sock"));
    }

    void setAndGetWindowSize()
    {
        ConfigManager cm(m_configPath);
        cm.setWindowSize("large");
        QCOMPARE(cm.windowSize(), QString("large"));
    }

    void setAndGetWindowPosition()
    {
        ConfigManager cm(m_configPath);
        cm.setWindowPos(100, 200);
        QCOMPARE(cm.windowPosX(), 100);
        QCOMPARE(cm.windowPosY(), 200);
    }

    void setAndGetAnimationMode()
    {
        ConfigManager cm(m_configPath);
        cm.setAnimationMode("classic");
        QCOMPARE(cm.animationMode(), QString("classic"));
    }

    void setAndGetAnimationPeriod()
    {
        ConfigManager cm(m_configPath);
        cm.setAnimationPeriodMs(500);
        QCOMPARE(cm.animationPeriodMs(), 500);
    }

    void persistsAfterSave()
    {
        {
            ConfigManager cm(m_configPath);
            cm.setWindowSize("medium");
            cm.setWindowPos(50, 60);
            cm.setAnimationMode("classic");
            cm.setAnimationPeriodMs(2000);
        }
        // Reload
        ConfigManager cm2(m_configPath);
        QCOMPARE(cm2.windowSize(), QString("medium"));
        QCOMPARE(cm2.windowPosX(), 50);
        QCOMPARE(cm2.windowPosY(), 60);
        QCOMPARE(cm2.animationMode(), QString("classic"));
        QCOMPARE(cm2.animationPeriodMs(), 2000);
    }

    void corruptFileFallsBackToDefaults()
    {
        // Write garbage to config file
        QFile f(m_configPath);
        f.open(QIODevice::WriteOnly);
        f.write("not json {{{");
        f.close();

        ConfigManager cm(m_configPath);
        QCOMPARE(cm.windowSize(), QString("small"));
        QCOMPARE(cm.animationMode(), QString("breathing"));
    }

    void clampsAnimationPeriod()
    {
        ConfigManager cm(m_configPath);
        cm.setAnimationPeriodMs(50);   // below min 200
        QCOMPARE(cm.animationPeriodMs(), 200);
        cm.setAnimationPeriodMs(9999); // above max 5000
        QCOMPARE(cm.animationPeriodMs(), 5000);
    }

    void rejectsInvalidWindowSize()
    {
        ConfigManager cm(m_configPath);
        cm.setWindowSize("huge"); // invalid
        QCOMPARE(cm.windowSize(), QString("small")); // unchanged default
    }

    void rejectsInvalidAnimationMode()
    {
        ConfigManager cm(m_configPath);
        cm.setAnimationMode("rainbow"); // invalid
        QCOMPARE(cm.animationMode(), QString("breathing")); // unchanged default
    }

    void setAndGetSocketPath()
    {
        ConfigManager cm(m_configPath);
        cm.setSocketPath("/tmp/custom.sock");
        QCOMPARE(cm.socketPath(), QString("/tmp/custom.sock"));
    }

    void socketPathPersists()
    {
        {
            ConfigManager cm(m_configPath);
            cm.setSocketPath("/tmp/persist.sock");
        }
        ConfigManager cm2(m_configPath);
        QCOMPARE(cm2.socketPath(), QString("/tmp/persist.sock"));
    }

    void rejectsEmptySocketPath()
    {
        ConfigManager cm(m_configPath);
        cm.setSocketPath("");
        QCOMPARE(cm.socketPath(), QString("/tmp/trafficlight4ai.sock")); // unchanged
    }

    void defaultAiTool()
    {
        ConfigManager cm(m_configPath);
        QCOMPARE(cm.aiTool(), QString("codex"));
    }

    void setAndGetAiTool()
    {
        ConfigManager cm(m_configPath);
        cm.setAiTool("claude-code");
        QCOMPARE(cm.aiTool(), QString("claude-code"));
    }

    void aiToolPersists()
    {
        {
            ConfigManager cm(m_configPath);
            cm.setAiTool("claude-code");
        }
        ConfigManager cm2(m_configPath);
        QCOMPARE(cm2.aiTool(), QString("claude-code"));
    }

    void defaultTimeoutSec()
    {
        ConfigManager cm(m_configPath);
        QCOMPARE(cm.timeoutSec(), 300);
    }

    void setAndGetTimeoutSec()
    {
        ConfigManager cm(m_configPath);
        cm.setTimeoutSec(600);
        QCOMPARE(cm.timeoutSec(), 600);
    }

    void timeoutSecZeroDisables()
    {
        ConfigManager cm(m_configPath);
        cm.setTimeoutSec(0);
        QCOMPARE(cm.timeoutSec(), 0);
    }

    void clampsTimeoutSec()
    {
        ConfigManager cm(m_configPath);
        cm.setTimeoutSec(10); // below min 30, not 0
        QCOMPARE(cm.timeoutSec(), 30);
        cm.setTimeoutSec(9999); // above max 3600
        QCOMPARE(cm.timeoutSec(), 3600);
    }

    void timeoutSecPersists()
    {
        {
            ConfigManager cm(m_configPath);
            cm.setTimeoutSec(120);
        }
        ConfigManager cm2(m_configPath);
        QCOMPARE(cm2.timeoutSec(), 120);
    }

    void normalizeInvalidWindowSize()
    {
        // Write valid JSON with invalid window.size
        QFile f(m_configPath);
        f.open(QIODevice::WriteOnly);
        f.write(R"({"window":{"size":"huge","posX":20,"posY":20}})");
        f.close();

        ConfigManager cm(m_configPath);
        QCOMPARE(cm.windowSize(), QString("small")); // normalized to default
    }

    void normalizeInvalidAnimationMode()
    {
        QFile f(m_configPath);
        f.open(QIODevice::WriteOnly);
        f.write(R"({"animation":{"mode":"rainbow","periodMs":1000}})");
        f.close();

        ConfigManager cm(m_configPath);
        QCOMPARE(cm.animationMode(), QString("breathing"));
    }

    void normalizeOutOfRangeAnimationPeriod()
    {
        QFile f(m_configPath);
        f.open(QIODevice::WriteOnly);
        f.write(R"({"animation":{"mode":"breathing","periodMs":99999}})");
        f.close();

        ConfigManager cm(m_configPath);
        QCOMPARE(cm.animationPeriodMs(), 5000); // clamped
    }

    void normalizeNegativeTimeout()
    {
        QFile f(m_configPath);
        f.open(QIODevice::WriteOnly);
        f.write(R"({"timeoutSec":-100})");
        f.close();

        ConfigManager cm(m_configPath);
        QCOMPARE(cm.timeoutSec(), 30); // clamped to min
    }
};

QTEST_MAIN(TestConfigManager)
#include "test_config_manager.moc"
