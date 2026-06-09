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
    // Priority: $XDG_RUNTIME_DIR, then $TMPDIR (macOS per-user), then /tmp
    const char *xdg = std::getenv("XDG_RUNTIME_DIR");
    if (xdg && xdg[0] != '\0')
        return QByteArray(xdg) + "/trafficlight4ai.sock";
    const char *tmpDir = std::getenv("TMPDIR");
    if (tmpDir && tmpDir[0] != '\0')
        return QByteArray(tmpDir) + "trafficlight4ai.sock";
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
    if (!socket.waitForConnected(100))
        return 0;

    socket.write(payload);
    socket.waitForBytesWritten(100);
    socket.disconnectFromServer();
    return 0;
}
