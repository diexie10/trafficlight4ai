#include "IpcServer.h"
#include "StateManager.h"
#include <QLocalSocket>
#include <QFile>

IpcServer::IpcServer(StateManager *stateManager, const QString &socketPath, QObject *parent)
    : QObject(parent), m_stateManager(stateManager), m_socketPath(socketPath)
{
    // Remove stale socket file if it exists
    if (QFile::exists(m_socketPath))
        QFile::remove(m_socketPath);

    connect(&m_server, &QLocalServer::newConnection, this, &IpcServer::onNewConnection);
    m_server.listen(m_socketPath);
}

IpcServer::~IpcServer()
{
    m_server.close();
    QFile::remove(m_socketPath);
}

bool IpcServer::isListening() const
{
    return m_server.isListening();
}

void IpcServer::restart(const QString &newPath)
{
    m_server.close();
    QFile::remove(m_socketPath);

    m_socketPath = newPath;
    if (QFile::exists(m_socketPath))
        QFile::remove(m_socketPath);
    m_server.listen(m_socketPath);
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
