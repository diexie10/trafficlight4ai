#include <QtTest>
#include <QLocalSocket>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "IpcServer.h"
#include "StateManager.h"

class TestIpcServer : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    int m_testIndex = 0;
    QString m_socketPath;

    void sendCommand(const QByteArray &data)
    {
        sendCommandTo(m_socketPath, data);
    }

    void sendCommandTo(const QString &path, const QByteArray &data)
    {
        QLocalSocket socket;
        socket.connectToServer(path);
        QVERIFY(socket.waitForConnected(1000));
        socket.write(data);
        socket.flush();
        socket.waitForBytesWritten(1000);
        socket.disconnectFromServer();
        QTest::qWait(100);
    }

    bool tryConnect(const QString &path)
    {
        QLocalSocket socket;
        socket.connectToServer(path);
        return socket.waitForConnected(200);
    }

private slots:
    void init()
    {
        m_socketPath = m_tempDir.path() + "/test_" + QString::number(m_testIndex++) + ".sock";
    }

    void serverStartsAndListens()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);
        QVERIFY(server.isListening());
    }

    void redCommandSetsWorking()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);

        sendCommand("RED\n");
        QCOMPARE(sm.state(), LightState::Working);
    }

    void yellowCommandSetsWaitingConfirm()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);

        sendCommand("YELLOW\n");
        QCOMPARE(sm.state(), LightState::WaitingConfirm);
    }

    void greenCommandSetsIdle()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);

        sm.setState(LightState::Working);
        sendCommand("GREEN\n");
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void invalidCommandIgnored()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);

        sendCommand("BLUE\n");
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void multipleClientsSequential()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);

        sendCommand("RED\n");
        QCOMPARE(sm.state(), LightState::Working);

        sendCommand("YELLOW\n");
        QCOMPARE(sm.state(), LightState::WaitingConfirm);

        sendCommand("GREEN\n");
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void doesNotDeleteRegularFile()
    {
        // A regular file at the socket path should NOT be deleted
        QFile f(m_socketPath);
        f.open(QIODevice::WriteOnly);
        f.write("not a socket");
        f.close();
        QVERIFY(QFile::exists(m_socketPath));

        StateManager sm;
        IpcServer server(&sm, m_socketPath);
        QVERIFY(!server.isListening()); // should fail to listen
        QVERIFY(QFile::exists(m_socketPath)); // file must still exist
    }

    void removesStaleSocketOnStart()
    {
        // Create a real stale Unix socket via POSIX API: bind but only close fd (no unlink)
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        QVERIFY(fd >= 0);
        struct sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, m_socketPath.toLocal8Bit().constData(), sizeof(addr.sun_path) - 1);
        QCOMPARE(bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)), 0);
        close(fd); // close fd without unlinking — leaves stale socket file

        QVERIFY(QFile::exists(m_socketPath));

        StateManager sm;
        IpcServer server(&sm, m_socketPath);
        QVERIFY(server.isListening());
    }

    void cleansUpSocketOnDestruction()
    {
        {
            StateManager sm;
            IpcServer server(&sm, m_socketPath);
            QVERIFY(server.isListening());
        }
        QVERIFY(!QFile::exists(m_socketPath));
    }

    void commandWithoutNewline()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);

        sendCommand("RED");
        QCOMPARE(sm.state(), LightState::Working);
    }

    void restartListensOnNewPath()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);
        QVERIFY(server.isListening());

        const QString newPath = m_tempDir.path() + "/restarted.sock";
        QVERIFY(server.restart(newPath));
        QVERIFY(server.isListening());

        sendCommandTo(newPath, "RED\n");
        QCOMPARE(sm.state(), LightState::Working);
    }

    void restartCleansOldSocket()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);
        const QString oldPath = m_socketPath;

        const QString newPath = m_tempDir.path() + "/restarted2.sock";
        QVERIFY(server.restart(newPath));

        QVERIFY(!QFile::exists(oldPath));
        QVERIFY(!tryConnect(oldPath));
    }

    void restartFailsKeepsOldSocket()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);
        QVERIFY(server.isListening());

        // Create a regular file at the new path — restart should fail
        const QString badPath = m_tempDir.path() + "/blocked.sock";
        QFile f(badPath);
        f.open(QIODevice::WriteOnly);
        f.write("not a socket");
        f.close();

        QVERIFY(!server.restart(badPath));
        // Old socket should still be working (rolled back)
        QVERIFY(server.isListening());
        QVERIFY(QFile::exists(m_socketPath));
        QVERIFY(QFile::exists(badPath));
        sendCommand("RED\n");
        QCOMPARE(sm.state(), LightState::Working);
    }
    void emptyCommandIgnored()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);

        sendCommand("\n");
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void commandCaseInsensitive()
    {
        StateManager sm;
        IpcServer server(&sm, m_socketPath);

        sendCommand("red\n");
        QCOMPARE(sm.state(), LightState::Working);
    }
};

QTEST_MAIN(TestIpcServer)
#include "test_ipc_server.moc"
