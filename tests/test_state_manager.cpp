#include <QtTest>
#include <QSignalSpy>
#include "StateManager.h"

class TestStateManager : public QObject {
    Q_OBJECT

private slots:
    void initialStateIsIdle()
    {
        StateManager sm;
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void setStateToWorking()
    {
        StateManager sm;
        sm.setState(LightState::Working);
        QCOMPARE(sm.state(), LightState::Working);
    }

    void setStateToWaitingConfirm()
    {
        StateManager sm;
        sm.setState(LightState::WaitingConfirm);
        QCOMPARE(sm.state(), LightState::WaitingConfirm);
    }

    void setStateToIdle()
    {
        StateManager sm;
        sm.setState(LightState::Working);
        sm.setState(LightState::Idle);
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void signalEmittedOnStateChange()
    {
        StateManager sm;
        QSignalSpy spy(&sm, &StateManager::stateChanged);
        sm.setState(LightState::Working);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<LightState>(), LightState::Working);
    }

    void noSignalOnSameState()
    {
        StateManager sm;
        sm.setState(LightState::Working);

        QSignalSpy spy(&sm, &StateManager::stateChanged);
        sm.setState(LightState::Working);
        QCOMPARE(spy.count(), 0);
    }

    void fullCycleTransitions()
    {
        StateManager sm;
        QSignalSpy spy(&sm, &StateManager::stateChanged);

        sm.setState(LightState::Working);
        sm.setState(LightState::WaitingConfirm);
        sm.setState(LightState::Idle);

        QCOMPARE(spy.count(), 3);
        QCOMPARE(spy.at(0).at(0).value<LightState>(), LightState::Working);
        QCOMPARE(spy.at(1).at(0).value<LightState>(), LightState::WaitingConfirm);
        QCOMPARE(spy.at(2).at(0).value<LightState>(), LightState::Idle);
    }

    void parseCommandRed()
    {
        StateManager sm;
        sm.handleCommand("RED");
        QCOMPARE(sm.state(), LightState::Working);
    }

    void parseCommandYellow()
    {
        StateManager sm;
        sm.handleCommand("YELLOW");
        QCOMPARE(sm.state(), LightState::WaitingConfirm);
    }

    void parseCommandGreen()
    {
        StateManager sm;
        sm.handleCommand("GREEN");
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void parseCommandInvalid()
    {
        StateManager sm;
        sm.handleCommand("BLUE");
        QCOMPARE(sm.state(), LightState::Idle); // unchanged
    }

    void parseCommandCaseInsensitive()
    {
        StateManager sm;
        sm.handleCommand("red");
        QCOMPARE(sm.state(), LightState::Working);
    }

    void parseCommandWithWhitespace()
    {
        StateManager sm;
        sm.handleCommand("  RED  \n");
        QCOMPARE(sm.state(), LightState::Working);
    }

    void workingTimesOutToIdle()
    {
        StateManager sm;
        sm.setTimeoutSec(1); // 1 second for fast test
        sm.setState(LightState::Working);
        // Wait for timeout + margin
        QTest::qWait(1500);
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void waitingConfirmTimesOutToIdle()
    {
        StateManager sm;
        sm.setTimeoutSec(1);
        sm.setState(LightState::WaitingConfirm);
        QTest::qWait(1500);
        QCOMPARE(sm.state(), LightState::Idle);
    }

    void idleDoesNotTimeout()
    {
        StateManager sm;
        sm.setTimeoutSec(1);
        // Already Idle, should stay Idle and not emit extra signals
        QSignalSpy spy(&sm, &StateManager::stateChanged);
        QTest::qWait(1500);
        QCOMPARE(sm.state(), LightState::Idle);
        QCOMPARE(spy.count(), 0);
    }

    void timeoutZeroDisables()
    {
        StateManager sm;
        sm.setTimeoutSec(0); // disabled
        sm.setState(LightState::Working);
        QTest::qWait(1500);
        QCOMPARE(sm.state(), LightState::Working); // still working, no timeout
    }

    void stateChangeResetsTimeout()
    {
        StateManager sm;
        sm.setTimeoutSec(1);
        sm.setState(LightState::Working);
        // Wait 700ms, then reset by switching state
        QTest::qWait(700);
        sm.setState(LightState::WaitingConfirm);
        // Wait another 700ms — less than 1s from last state change
        QTest::qWait(700);
        QCOMPARE(sm.state(), LightState::WaitingConfirm); // not timed out yet
        // Wait remainder
        QTest::qWait(800);
        QCOMPARE(sm.state(), LightState::Idle); // now timed out
    }

    void duplicateCommandRefreshesTimeout()
    {
        StateManager sm;
        sm.setTimeoutSec(1);
        sm.setState(LightState::Working);
        // Wait 700ms, send same state again — should refresh timeout
        QTest::qWait(700);
        sm.setState(LightState::Working); // duplicate, but should reset timer
        // Wait another 700ms — only 700ms since refresh, should not timeout
        QTest::qWait(700);
        QCOMPARE(sm.state(), LightState::Working); // still working
        // Wait for the full timeout from last refresh
        QTest::qWait(800);
        QCOMPARE(sm.state(), LightState::Idle); // now timed out
    }

    void duplicateCommandDoesNotEmitSignal()
    {
        StateManager sm;
        sm.setState(LightState::Working);
        QSignalSpy spy(&sm, &StateManager::stateChanged);
        sm.setState(LightState::Working); // duplicate
        QCOMPARE(spy.count(), 0); // no signal for same state
    }

    void setTimeoutZeroWhileWorkingCancelsTimer()
    {
        StateManager sm;
        sm.setTimeoutSec(1);
        sm.setState(LightState::Working);
        // Disable timeout while working
        sm.setTimeoutSec(0);
        QTest::qWait(1500);
        QCOMPARE(sm.state(), LightState::Working); // timer was cancelled
    }
};

QTEST_MAIN(TestStateManager)
#include "test_state_manager.moc"
