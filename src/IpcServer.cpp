#include "IpcServer.h"
#include "StateManager.h"
#include <QLocalSocket>
#include <QFile>
#include <QFileInfo>

static bool isSocketFile(const QString &path)
{
    if (!QFile::exists(path))
        return false;
    QFileInfo fi(path);
    // QLocalServer creates socket files; regular files should not be deleted
    return fi.isFile() || fi.isSymLink()
           || (!fi.isDir() && QFile::exists(path));
}

static void removeStaleSocket(const QString &path)
{
    // Try to connect — if it succeeds, another instance is using it
    QLocalSocket probe;
    probe.connectToServer(path);
    if (probe.waitForConnected(100)) {
        probe.disconnectFromServer();
        return; // socket is live, don't remove
    }
    // Stale socket or leftover file — safe to remove
    QLocalServer::removeServer(path);
}

IpcServer::IpcServer(StateManager *stateManager, const QString &socketPath, QObject *parent)
    : QObject(parent), m_stateManager(stateManager), m_socketPath(socketPath)
{
    removeStaleSocket(m_socketPath);

    connect(&m_server, &QLocalServer::newConnection, this, &IpcServer::onNewConnection);
    if (!m_server.listen(m_socketPath))
        qWarning("IpcServer: failed to listen on %s: %s",
                 qPrintable(m_socketPath), qPrintable(m_server.errorString()));
}

IpcServer::~IpcServer()
{
    m_server.close();
    QLocalServer::removeServer(m_socketPath);
}

bool IpcServer::isListening() const
{
    return m_server.isListening();
}

void IpcServer::restart(const QString &newPath)
{
    m_server.close();
    QLocalServer::removeServer(m_socketPath);

    m_socketPath = newPath;
    removeStaleSocket(m_socketPath);
    if (!m_server.listen(m_socketPath))
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
