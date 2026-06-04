#include "IpcServer.h"
#include "StateManager.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QFile>
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
    : QObject(parent), m_server(std::make_unique<QLocalServer>()),
      m_stateManager(stateManager), m_socketPath(socketPath)
{
    removeStaleSocket(m_socketPath);

    connectServer();
    m_ownsSocket = m_server->listen(m_socketPath);
    if (!m_ownsSocket)
        qWarning("IpcServer: failed to listen on %s: %s",
                 qPrintable(m_socketPath), qPrintable(m_server->errorString()));
}

IpcServer::~IpcServer()
{
    m_server->close();
    if (m_ownsSocket)
        QFile::remove(m_socketPath);
}

bool IpcServer::isListening() const
{
    return m_server && m_server->isListening();
}

bool IpcServer::restart(const QString &newPath)
{
    if (newPath == m_socketPath)
        return isListening();

    auto newServer = std::make_unique<QLocalServer>();

    removeStaleSocket(newPath);
    if (!newServer->listen(newPath)) {
        qWarning("IpcServer: failed to listen on %s: %s",
                 qPrintable(newPath), qPrintable(newServer->errorString()));
        return false;
    }

    const QString oldPath = m_socketPath;
    const bool oldOwned = m_ownsSocket;

    m_server->close();
    if (oldOwned)
        QFile::remove(oldPath);

    m_server = std::move(newServer);
    m_socketPath = newPath;
    m_ownsSocket = true;
    connectServer();
    return true;
}

void IpcServer::connectServer()
{
    connect(m_server.get(), &QLocalServer::newConnection, this, &IpcServer::onNewConnection);
}

void IpcServer::onNewConnection()
{
    while (QLocalSocket *client = m_server->nextPendingConnection()) {
        auto processData = [this, client]() {
            QByteArray data = client->readAll();
            if (!data.isEmpty())
                m_stateManager->handleCommand(QString::fromUtf8(data));
            client->disconnectFromServer();
            client->deleteLater();
        };

        connect(client, &QLocalSocket::disconnected, client, &QObject::deleteLater);

        // Data may arrive before readyRead is connected (fast CLI sender).
        // Check immediately; otherwise wait for the signal. Mutually exclusive.
        if (client->bytesAvailable()) {
            processData();
        } else {
            connect(client, &QLocalSocket::readyRead, this, processData);
        }
    }
}
