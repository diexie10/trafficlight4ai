#include "StateManager.h"

StateManager::StateManager(QObject *parent)
    : QObject(parent)
{
    m_timeoutTimer.setSingleShot(true);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &StateManager::onTimeout);
}

LightState StateManager::state() const
{
    return m_state;
}

void StateManager::setState(LightState newState)
{
    if (m_state == newState)
        return;
    m_state = newState;
    emit stateChanged(m_state);

    if (m_state == LightState::Idle)
        stopTimer();
    else
        restartTimer();
}

void StateManager::handleCommand(const QString &command)
{
    const QString cmd = command.trimmed().toUpper();
    if (cmd == "RED")
        setState(LightState::Working);
    else if (cmd == "YELLOW")
        setState(LightState::WaitingConfirm);
    else if (cmd == "GREEN")
        setState(LightState::Idle);
    // unknown commands silently ignored
}

int StateManager::timeoutSec() const
{
    return m_timeoutSec;
}

void StateManager::setTimeoutSec(int sec)
{
    m_timeoutSec = sec;
    // Restart timer if currently in a non-idle state
    if (m_state != LightState::Idle)
        restartTimer();
}

void StateManager::onTimeout()
{
    setState(LightState::Idle);
}

void StateManager::restartTimer()
{
    if (m_timeoutSec > 0)
        m_timeoutTimer.start(m_timeoutSec * 1000);
    else
        stopTimer();
}

void StateManager::stopTimer()
{
    m_timeoutTimer.stop();
}
