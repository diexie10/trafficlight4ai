#pragma once

#include <QObject>
#include <QTimer>

enum class LightState {
    Working,        // Red blinking
    WaitingConfirm, // Yellow blinking
    Idle            // Green steady
};

Q_DECLARE_METATYPE(LightState)

class StateManager : public QObject {
    Q_OBJECT

public:
    explicit StateManager(QObject *parent = nullptr);

    LightState state() const;
    void setState(LightState newState);
    void handleCommand(const QString &command);

    int timeoutSec() const;
    void setTimeoutSec(int sec);

signals:
    void stateChanged(LightState newState);

private slots:
    void onTimeout();

private:
    void restartTimer();
    void stopTimer();

    LightState m_state = LightState::Idle;
    QTimer m_timeoutTimer;
    int m_timeoutSec = 0;
};
