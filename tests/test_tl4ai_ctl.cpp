#include <QtTest>
#include <QProcess>
#include <QCoreApplication>
#include "IpcServer.h"
#include "StateManager.h"

class TestTl4aiCtl : public QObject {
    Q_OBJECT

private:
    QString m_ctlPath;
    QTemporaryDir m_tempDir;

    QString socketPath() const
    {
        return m_tempDir.path() + "/test.sock";
    }

    int runCtl(const QStringList &args, const QString &sockPath)
    {
        QProcess proc;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("TL4AI_SOCKET", sockPath);
        proc.setProcessEnvironment(env);
        proc.start(m_ctlPath, args);
        proc.waitForFinished(2000);
        return proc.exitCode();
    }

private slots:
    void initTestCase()
    {
        // Find the tl4ai-ctl binary relative to test binary
        m_ctlPath = QCoreApplication::applicationDirPath() + "/../tools/tl4ai-ctl";
        if (!QFile::exists(m_ctlPath)) {
            QSKIP("tl4ai-ctl binary not found, build it first");
        }
    }

    void sendsRedCommand()
    {
        StateManager sm;
        IpcServer server(&sm, socketPath());

        QCOMPARE(runCtl({"red"}, socketPath()), 0);
        QTest::qWait(200);
        QCOMPARE(sm.state(), LightState::Working);
    }

    void sendsYellowCommand()
    {
        StateManager sm;
        IpcServer server(&sm, socketPath());

        QCOMPARE(runCtl({"yellow"}, socketPath()), 0);
        QTest::qWait(200);
        QCOMPARE(sm.state(), LightState::WaitingConfirm);
    }

    void sendsGreenCommand()
    {
        StateManager sm;
        IpcServer server(&sm, socketPath());

        sm.setState(LightState::Working);
        QCOMPARE(runCtl({"green"}, socketPath()), 0);
        QTest::qWait(200);
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void exitZeroWhenServerNotRunning()
    {
        // No server listening - should still exit 0 silently
        QCOMPARE(runCtl({"red"}, "/tmp/nonexistent_tl4ai_test.sock"), 0);
    }

    void exitZeroWithNoArgs()
    {
        // No arguments - should exit 0 without crashing
        QCOMPARE(runCtl({}, socketPath()), 0);
    }
};

QTEST_MAIN(TestTl4aiCtl)
#include "test_tl4ai_ctl.moc"
