#include "IpcServer.h"
#include "StateManager.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QFile>
#include <QtGlobal>
#ifdef Q_OS_UNIX
#include <sys/stat.h>
#endif

#ifdef Q_OS_UNIX
static bool isUnixSocket(const QString &path)
{
    struct stat st;
    if (stat(path.toLocal8Bit().constData(), &st) != 0)
        return false;
    return S_ISSOCK(st.st_mode);
}
#endif

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
    if (isLiveSocket(path))
        return;

#ifdef Q_OS_UNIX
    if (!QFile::exists(path))
        return;
    // Only remove actual Unix sockets, never regular files
    if (!isUnixSocket(path))
        return;

    QFile::remove(path);
#else
    QLocalServer::removeServer(path);
#endif
}

static void removeOwnedServer(const QString &path)
{
#ifdef Q_OS_UNIX
    QFile::remove(path);
#else
    QLocalServer::removeServer(path);
#endif
}

IpcServer::IpcServer(StateManager *stateManager, const QString &socketPath, QObject *parent)
    : QObject(parent), m_server(std::make_unique<QLocalServer>()),
      m_stateManager(stateManager), m_socketPath(socketPath)
{
    removeStaleSocket(m_socketPath);

    m_server->setSocketOptions(QLocalServer::UserAccessOption);
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
        removeOwnedServer(m_socketPath);
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
    newServer->setSocketOptions(QLocalServer::UserAccessOption);

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
        removeOwnedServer(oldPath);

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
        auto *buf = new QByteArray();

        auto finishClient = [this, client, buf]() {
            // Disconnect all signals first to prevent re-entry
            client->disconnect(this);
            client->disconnect(client);
            if (!buf->isEmpty())
                m_stateManager->handleCommand(QString::fromUtf8(*buf));
            delete buf;
            client->disconnectFromServer();
            client->deleteLater();
        };

        auto tryProcess = [this, client, buf, finishClient]() {
            buf->append(client->read(64 - buf->size()));
            if (buf->contains('\n') || buf->size() >= 64)
                finishClient();
        };

        if (client->bytesAvailable()) {
            buf->append(client->read(64));
            if (buf->contains('\n') || buf->size() >= 64) {
                m_stateManager->handleCommand(QString::fromUtf8(*buf));
                delete buf;
                client->disconnectFromServer();
                client->deleteLater();
            } else {
                connect(client, &QLocalSocket::readyRead, this, tryProcess);
                connect(client, &QLocalSocket::disconnected, this, finishClient);
            }
        } else {
            connect(client, &QLocalSocket::readyRead, this, tryProcess);
            connect(client, &QLocalSocket::disconnected, this, finishClient);
        }
    }
}
