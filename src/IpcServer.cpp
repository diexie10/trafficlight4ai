#include "IpcServer.h"
#include "StateManager.h"
#include <QLocalSocket>
#include <QFile>
#include <QFileInfo>
#include <sys/stat.h>

static bool isUnixSocket(const QString &path)
{
    struct stat st;
    if (stat(path.toLocal8Bit().constData(), &st) != 0)
        return false;
    return S_ISSOCK(st.st_mode);
}

static bool isLiveSocket(const QString &path)
{
    QLocalSocket probe;
    probe.connectToServer(path);
    if (probe.waitForConnected(100)) {
        probe.disconnectFromServer();
        return true;
    }
    return false;
}

static void removeStaleSocket(const QString &path)
{
    if (!QFile::exists(path))
        return;

    // Only remove actual Unix sockets, never regular files
    if (!isUnixSocket(path))
        return;

    // Don't remove if another instance is actively listening
    if (isLiveSocket(path))
        return;

    QFile::remove(path);
}

IpcServer::IpcServer(StateManager *stateManager, const QString &socketPath, QObject *parent)
    : QObject(parent), m_stateManager(stateManager), m_socketPath(socketPath)
{
    removeStaleSocket(m_socketPath);

    connect(&m_server, &QLocalServer::newConnection, this, &IpcServer::onNewConnection);
    m_ownsSocket = m_server.listen(m_socketPath);
    if (!m_ownsSocket)
        qWarning("IpcServer: failed to listen on %s: %s",
                 qPrintable(m_socketPath), qPrintable(m_server.errorString()));
}

IpcServer::~IpcServer()
{
    m_server.close();
    if (m_ownsSocket)
        QFile::remove(m_socketPath);
}

bool IpcServer::isListening() const
{
    return m_server.isListening();
}

void IpcServer::restart(const QString &newPath)
{
    m_server.close();
    if (m_ownsSocket)
        QFile::remove(m_socketPath);

    m_socketPath = newPath;
    removeStaleSocket(m_socketPath);
    m_ownsSocket = m_server.listen(m_socketPath);
    if (!m_ownsSocket)
        qWarning("IpcServer: failed to listen on %s: %s",
                 qPrintable(m_socketPath), qPrintable(m_server.errorString()));
}

void IpcServer::onNewConnection()
{
    while (QLocalSocket *client = m_server.nextPendingConnection()) {
        connect(client, &QLocalSocket::readyRead, this, [this, client]() {
            QByteArray data = client->readAll();
            if (!data.isEmpty())
                m_stateManager->handleCommand(QString::fromUtf8(data));
            client->disconnectFromServer();
            client->deleteLater();
        });
        connect(client, &QLocalSocket::disconnected, client, &QObject::deleteLater);

        // Handle data already available at connection time
        if (client->bytesAvailable()) {
            QByteArray data = client->readAll();
            if (!data.isEmpty())
                m_stateManager->handleCommand(QString::fromUtf8(data));
            client->disconnectFromServer();
            client->deleteLater();
        }
    }
}
