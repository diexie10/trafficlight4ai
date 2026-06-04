#pragma once

#include <QObject>
#include <memory>

class QLocalServer;
class StateManager;

class IpcServer : public QObject {
    Q_OBJECT

public:
    explicit IpcServer(StateManager *stateManager, const QString &socketPath,
                       QObject *parent = nullptr);
    ~IpcServer() override;

    bool isListening() const;
    bool restart(const QString &newPath);

private slots:
    void onNewConnection();

private:
    void connectServer();

    std::unique_ptr<QLocalServer> m_server;
    StateManager *m_stateManager;
    QString m_socketPath;
    bool m_ownsSocket = false; // true only if this instance successfully listen()ed
};
