#include <QtTest>
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
};

QTEST_MAIN(TestTrafficLightWidget)
#include "test_traffic_light_widget.moc"
