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

    void stateChangesEmitAlpha()
    {
        // All three curve animations run continuously regardless of state;
        // activeAlpha breathes whether Working, WaitingConfirm, or Idle.
        TrafficLightWidget w;
        QSignalSpy spy(&w, &TrafficLightWidget::activeAlphaChanged);

        w.onStateChanged(LightState::Working);
        QTRY_VERIFY_WITH_TIMEOUT(!spy.isEmpty(), 2000);

        w.onStateChanged(LightState::Idle);
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 2, 2000);
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

    void stateChangeCausesVisualDifference()
    {
        // Verify that state changes are visually reflected in the rendered output.
        TrafficLightWidget w;
        w.setSizePreset(TrafficLightWidget::Medium);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));

        // Force initial state to Idle and wait for fade to settle.
        w.onStateChanged(LightState::Idle);
        QTest::qWait(400);

        // Grab the left-third region where the red slot sits.
        const int cw = w.width();
        const int ch = w.height();
        const QRect leftThird(0, 0, cw / 3, ch);

        // Snapshot in Idle state (left slot inactive → dim).
        const QImage idleImg = w.grab().copy(leftThird).toImage();

        // Switch to Working and wait for full fade.
        w.onStateChanged(LightState::Working);
        QTest::qWait(400);

        const QImage workImg = w.grab().copy(leftThird).toImage();

        // Compute average brightness of the left region in both states.
        // During Idle the left slot is dim (inactive), during Working it's bright.
        double sumIdle = 0, sumWork = 0;
        const int pixels = idleImg.width() * idleImg.height();
        for (int y = 0; y < idleImg.height(); ++y) {
            for (int x = 0; x < idleImg.width(); ++x) {
                sumIdle += qGray(idleImg.pixel(x, y));
                sumWork += qGray(workImg.pixel(x, y));
            }
        }
        const double avgIdle = sumIdle / pixels;
        const double avgWork = sumWork / pixels;
        const double diff = qAbs(avgWork - avgIdle);

        qDebug() << "[Test] Left region avg brightness: idle=" << avgIdle
                 << " working=" << avgWork << " diff=" << diff;
        QVERIFY2(diff > 5.0,
                 "Left region brightness barely changed between Idle and Working");
    }
};

QTEST_MAIN(TestTrafficLightWidget)
#include "test_traffic_light_widget.moc"
