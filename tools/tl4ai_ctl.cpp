#include <QByteArray>
#include <QCoreApplication>
#include <QLocalSocket>
#include <QString>

#include <cstdlib>
#include <string>

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

static QByteArray computeDefaultSocketPath()
{
#ifdef _WIN32
    return QByteArrayLiteral("trafficlight4ai");
#else
    // Priority: $XDG_RUNTIME_DIR, then $TMPDIR (macOS only), then /tmp
    const char *xdg = std::getenv("XDG_RUNTIME_DIR");
    if (xdg && xdg[0] != '\0')
        return QByteArray(xdg) + "/trafficlight4ai.sock";
#ifdef __APPLE__
    const char *tmpDir = std::getenv("TMPDIR");
    if (tmpDir && tmpDir[0] != '\0') {
        QByteArray dir(tmpDir);
        if (!dir.endsWith('/'))
            dir += '/';
        return dir + "trafficlight4ai.sock";
    }
#endif
    return QByteArray("/tmp/trafficlight4ai-") + QByteArray::number(getuid()) + ".sock";
#endif
}

static void drainStdin()
{
#ifndef _WIN32
    // Drain stdin non-blocking to avoid broken pipe from callers (e.g. Codex hooks)
    int stdinFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (stdinFlags < 0)
        return;
    fcntl(STDIN_FILENO, F_SETFL, stdinFlags | O_NONBLOCK);
    char buf[4096];
    while (read(STDIN_FILENO, buf, sizeof(buf)) > 0) {}
    fcntl(STDIN_FILENO, F_SETFL, stdinFlags);
#endif
}

int main(int argc, char *argv[])
{
    drainStdin();

    if (argc < 2)
        return 0;

    QCoreApplication app(argc, argv);

    // Determine socket name/path: env var > default.
    QByteArray socketPath = qgetenv("TL4AI_SOCKET");
    if (socketPath.isEmpty())
        socketPath = computeDefaultSocketPath();

    QString command = QString::fromLocal8Bit(argv[1]).toUpper();
    command += '\n';
    const QByteArray payload = command.toUtf8();

    QLocalSocket socket;
    socket.connectToServer(QString::fromLocal8Bit(socketPath));
    // Windows named pipe 需要更长时间建立连接
    if (!socket.waitForConnected(1000)) {
        qWarning("tl4ai-ctl: cannot connect to server at %s",
                  socketPath.constData());
        return 1; // 返回非零，让调用方知道失败了
    }

    const qint64 written = socket.write(payload);
    if (written != payload.size() || !socket.waitForBytesWritten(1000)) {
        qWarning("tl4ai-ctl: failed to write command to server");
        socket.disconnectFromServer();
        return 1;
    }
    socket.disconnectFromServer();
    return 0;
}
