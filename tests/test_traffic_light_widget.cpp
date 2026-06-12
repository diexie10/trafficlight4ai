#include <QtTest>
#include <QSignalSpy>
#include "TrafficLightWidget.h"

class TestTrafficLightWidget : public QObject {
    Q_OBJECT

private slots:
    void sizePresetFromStringXSmall()
    {
        QCOMPARE(TrafficLightWidget::sizePresetFromString("xsmall"), TrafficLightWidget::ExtraSmall);
    }

    void sizePresetFromStringSmall()
    {
        QCOMPARE(TrafficLightWidget::sizePresetFromString("small"), TrafficLightWidget::Small);
    }

    void sizePresetFromStringMedium()
    {
        QCOMPARE(TrafficLightWidget::sizePresetFromString("medium"), TrafficLightWidget::Medium);
    }

    void sizePresetFromStringLarge()
    {
        QCOMPARE(TrafficLightWidget::sizePresetFromString("large"), TrafficLightWidget::Large);
    }

    void sizePresetFromStringXLarge()
    {
        QCOMPARE(TrafficLightWidget::sizePresetFromString("xlarge"), TrafficLightWidget::ExtraLarge);
    }

    void sizePresetFromStringInvalid()
    {
        QCOMPARE(TrafficLightWidget::sizePresetFromString("huge"), TrafficLightWidget::Small);
    }

    void sizePresetFromStringEmpty()
    {
        QCOMPARE(TrafficLightWidget::sizePresetFromString(""), TrafficLightWidget::Small);
    }

    void stateChangedToWorkingEmitsAlpha()
    {
        TrafficLightWidget w;
        QSignalSpy spy(&w, &TrafficLightWidget::activeAlphaChanged);
        w.onStateChanged(LightState::Working);
        QTRY_VERIFY_WITH_TIMEOUT(!spy.isEmpty(), 2000);
    }

    void stateChangedToIdleStopsAnimation()
    {
        TrafficLightWidget w;
        w.onStateChanged(LightState::Working);
        QTest::qWait(100);

        w.onStateChanged(LightState::Idle);
        QTest::qWait(500);
        QCOMPARE(w.activeAlpha(), 1.0);
    }

    void activeAlphaRoundTrip()
    {
        TrafficLightWidget w;
        w.setActiveAlpha(0.5);
        QCOMPARE(w.activeAlpha(), 0.5);
    }

    void classicAnimationCanRestartWhileActive()
    {
        TrafficLightWidget w;
        QSignalSpy spy(&w, &TrafficLightWidget::activeAlphaChanged);

        w.onStateChanged(LightState::Working);
        w.setAnimationMode("classic");
        w.setAnimationPeriodMs(300);

        QTRY_VERIFY_WITH_TIMEOUT(!spy.isEmpty(), 1000);
    }

    void paintEventRendersAllStates()
    {
        TrafficLightWidget w;
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));

        QVERIFY(!w.grab().isNull());

        w.onStateChanged(LightState::Working);
        QVERIFY(!w.grab().isNull());

        w.onStateChanged(LightState::WaitingConfirm);
        QVERIFY(!w.grab().isNull());

        w.onStateChanged(LightState::Idle);
        QVERIFY(!w.grab().isNull());
    }
};

QTEST_MAIN(TestTrafficLightWidget)
#include "test_traffic_light_widget.moc"
