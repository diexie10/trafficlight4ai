#include <QtTest>
#include <QAction>
#include <QMenu>
#include <QTemporaryDir>
#include "ConfigManager.h"
#include "FloatingWindow.h"
#include "IpcServer.h"
#include "SettingsDialog.h"
#include "StateManager.h"
#include "TrafficLightWidget.h"
#include "TrayIcon.h"

class TestTrayIcon : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    int m_testIndex = 0;

    QString configPath()
    {
        return m_tempDir.path() + "/config_" + QString::number(m_testIndex++) + ".json";
    }

    static QAction *requireAction(QMenu *menu, const char *name)
    {
        auto *action = menu->findChild<QAction *>(name);
        if (!action)
            qFatal("Missing menu action: %s", name);
        return action;
    }

private slots:
    void constructCreatesIconAndMenu()
    {
        ConfigManager config(configPath());
        StateManager stateManager;
        IpcServer ipcServer(&stateManager, m_tempDir.path() + "/tray_construct.sock");
        auto *light = new TrafficLightWidget();
        FloatingWindow window(light, &config);
        SettingsDialog settings(&config, light, &ipcServer, &stateManager);

        TrayIcon tray(&window, &settings);

        QVERIFY(!tray.icon().isNull());
        QVERIFY(tray.contextMenu() != nullptr);
    }

    void menuActionsToggleWindowAndShowSettings()
    {
        ConfigManager config(configPath());
        StateManager stateManager;
        IpcServer ipcServer(&stateManager, m_tempDir.path() + "/tray_actions.sock");
        auto *light = new TrafficLightWidget();
        FloatingWindow window(light, &config);
        SettingsDialog settings(&config, light, &ipcServer, &stateManager);
        TrayIcon tray(&window, &settings);

        auto *menu = tray.contextMenu();
        QVERIFY(menu != nullptr);

        QVERIFY(!window.isVisible());
        requireAction(menu, "toggleWindowAction")->trigger();
        QVERIFY(window.isVisible());

        QVERIFY(!settings.isVisible());
        requireAction(menu, "settingsAction")->trigger();
        QVERIFY(settings.isVisible());
    }

    void aiToolChangedUpdatesTooltip()
    {
        ConfigManager config(configPath());
        StateManager stateManager;
        IpcServer ipcServer(&stateManager, m_tempDir.path() + "/tray_tooltip.sock");
        auto *light = new TrafficLightWidget();
        FloatingWindow window(light, &config);
        SettingsDialog settings(&config, light, &ipcServer, &stateManager);
        TrayIcon tray(&window, &settings);

        tray.onAiToolChanged("Gemini");

        QVERIFY(tray.toolTip().contains("Gemini"));
    }

    void stateChangesKeepValidIcon()
    {
        ConfigManager config(configPath());
        StateManager stateManager;
        IpcServer ipcServer(&stateManager, m_tempDir.path() + "/tray_states.sock");
        auto *light = new TrafficLightWidget();
        FloatingWindow window(light, &config);
        SettingsDialog settings(&config, light, &ipcServer, &stateManager);
        TrayIcon tray(&window, &settings);

        tray.onStateChanged(LightState::Working);
        QVERIFY(!tray.icon().isNull());
        tray.onStateChanged(LightState::WaitingConfirm);
        QVERIFY(!tray.icon().isNull());
        tray.onStateChanged(LightState::Idle);
        QTest::qWait(200);
        QVERIFY(!tray.icon().isNull());
    }

    void activeAlphaChangesKeepValidIcon()
    {
        ConfigManager config(configPath());
        StateManager stateManager;
        IpcServer ipcServer(&stateManager, m_tempDir.path() + "/tray_alpha.sock");
        auto *light = new TrafficLightWidget();
        FloatingWindow window(light, &config);
        SettingsDialog settings(&config, light, &ipcServer, &stateManager);
        TrayIcon tray(&window, &settings);

        tray.onStateChanged(LightState::Working);
        tray.onActiveAlphaChanged(0.35);
        QVERIFY(!tray.icon().isNull());

        tray.onStateChanged(LightState::Idle);
        tray.onActiveAlphaChanged(0.35);
        QVERIFY(!tray.icon().isNull());
    }

    void retranslateKeepsAiToolTooltip()
    {
        ConfigManager config(configPath());
        StateManager stateManager;
        IpcServer ipcServer(&stateManager, m_tempDir.path() + "/tray_retranslate.sock");
        auto *light = new TrafficLightWidget();
        FloatingWindow window(light, &config);
        SettingsDialog settings(&config, light, &ipcServer, &stateManager);
        TrayIcon tray(&window, &settings);

        tray.onAiToolChanged("Gemini");
        tray.retranslateUi();

        QVERIFY(tray.toolTip().contains("Gemini"));
    }

    void activationTriggerTogglesWindow()
    {
        ConfigManager config(configPath());
        StateManager stateManager;
        IpcServer ipcServer(&stateManager, m_tempDir.path() + "/tray_activation.sock");
        auto *light = new TrafficLightWidget();
        FloatingWindow window(light, &config);
        SettingsDialog settings(&config, light, &ipcServer, &stateManager);
        TrayIcon tray(&window, &settings);

        QVERIFY(!window.isVisible());
        QVERIFY(QMetaObject::invokeMethod(&tray, "onActivated",
                                          Q_ARG(QSystemTrayIcon::ActivationReason,
                                                QSystemTrayIcon::Trigger)));
        QVERIFY(window.isVisible());
    }
};

QTEST_MAIN(TestTrayIcon)
#include "test_tray_icon.moc"
