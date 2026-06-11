#include <QtTest>
#include <QTemporaryDir>
#include <QMouseEvent>
#include "ConfigManager.h"
#include "FloatingWindow.h"
#include "IpcServer.h"
#include "SettingsDialog.h"
#include "StateManager.h"
#include "TrafficLightWidget.h"

class TestFloatingWindow : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    int m_testIndex = 0;

    QString configPath()
    {
        return m_tempDir.path() + "/config_" + QString::number(m_testIndex++) + ".json";
    }

    static void sendMouseEvent(QWidget &widget, QEvent::Type type, const QPointF &localPos,
                               const QPointF &globalPos, Qt::MouseButton button,
                               Qt::MouseButtons buttons)
    {
        QMouseEvent event(type, localPos, localPos, globalPos, button, buttons, Qt::NoModifier);
        QApplication::sendEvent(&widget, &event);
    }

private slots:
    void restoresWindowPositionFromConfig()
    {
        ConfigManager config(configPath());
        config.setWindowPos(123, 234);
        auto *light = new TrafficLightWidget();

        FloatingWindow window(light, &config);

        QCOMPARE(window.pos(), QPoint(123, 234));
    }

    void leftDragReleaseStoresNewPosition()
    {
        ConfigManager config(configPath());
        config.setWindowPos(30, 40);
        auto *light = new TrafficLightWidget();
        FloatingWindow window(light, &config);

        const QPointF localPos(5, 5);
        sendMouseEvent(window, QEvent::MouseButtonPress, localPos, QPointF(35, 45),
                       Qt::LeftButton, Qt::LeftButton);
        sendMouseEvent(window, QEvent::MouseMove, localPos, QPointF(105, 125),
                       Qt::NoButton, Qt::LeftButton);
        sendMouseEvent(window, QEvent::MouseButtonRelease, localPos, QPointF(105, 125),
                       Qt::LeftButton, Qt::NoButton);

        QCOMPARE(window.pos(), QPoint(100, 120));
        QCOMPARE(config.windowPosX(), 100);
        QCOMPARE(config.windowPosY(), 120);
    }

    void settingsDialogCanBeAttachedWithoutChangingBasicBehavior()
    {
        ConfigManager config(configPath());
        config.setWindowPos(10, 20);
        StateManager stateManager;
        IpcServer ipcServer(&stateManager, m_tempDir.path() + "/floating_window.sock");
        auto *light = new TrafficLightWidget();
        FloatingWindow window(light, &config);
        SettingsDialog dialog(&config, light, &ipcServer, &stateManager);

        const QPoint before = window.pos();
        window.setSettingsDialog(&dialog);

        QCOMPARE(window.pos(), before);
        QVERIFY(window.layout() != nullptr);
    }
};

QTEST_MAIN(TestFloatingWindow)
#include "test_floating_window.moc"
