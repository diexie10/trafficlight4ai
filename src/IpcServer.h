#pragma once

#include <QObject>
#include <QLocalServer>

class StateManager;

class IpcServer : public QObject {
    Q_OBJECT

public:
    explicit IpcServer(StateManager *stateManager, const QString &socketPath,
                       QObject *parent = nullptr);
    ~IpcServer() override;

    bool isListening() const;
    void restart(const QString &newPath);

private slots:
    void onNewConnection();

private:
    QLocalServer m_server;
    StateManager *m_stateManager;
    QString m_socketPath;
    bool m_ownsSocket = false; // true only if this instance successfully listen()ed
};
