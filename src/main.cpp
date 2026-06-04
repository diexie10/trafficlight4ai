#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include "StateManager.h"
#include "ConfigManager.h"
#include "IpcServer.h"
#include "TrafficLightWidget.h"
#include "FloatingWindow.h"
#include "TrayIcon.h"
#include "SettingsDialog.h"
#include "AiToolStrategy.h"

static QString defaultConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
           + "/trafficlight4ai/config.json";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("trafficlight4ai");
    app.setQuitOnLastWindowClosed(false); // keep running in tray

    // Core
    ConfigManager config(defaultConfigPath());
    StateManager stateManager;
    stateManager.setTimeoutSec(config.timeoutSec());
    IpcServer ipcServer(&stateManager, config.socketPath());

    // GUI
    auto *lightWidget = new TrafficLightWidget();
    lightWidget->setAnimationMode(config.animationMode());
    lightWidget->setAnimationPeriodMs(config.animationPeriodMs());

    const QString size = config.windowSize();
    if (size == "medium")
        lightWidget->setSizePreset(TrafficLightWidget::Medium);
    else if (size == "large")
        lightWidget->setSizePreset(TrafficLightWidget::Large);

    auto *floatingWindow = new FloatingWindow(lightWidget, &config);
    floatingWindow->show();

    // Settings dialog
    auto *settingsDialog = new SettingsDialog(&config, lightWidget, &ipcServer, &stateManager);
    floatingWindow->setSettingsDialog(settingsDialog);

    // Tray icon with dynamic tooltip
    auto *trayIcon = new TrayIcon(floatingWindow, settingsDialog);
    if (auto *strategy = AiToolRegistry::find(config.aiTool()))
        trayIcon->setToolTip("Traffic Light for " + strategy->displayName());
    trayIcon->show();

    // Connect state changes to UI
    QObject::connect(&stateManager, &StateManager::stateChanged,
                     lightWidget, &TrafficLightWidget::onStateChanged);
    QObject::connect(&stateManager, &StateManager::stateChanged,
                     trayIcon, &TrayIcon::onStateChanged);

    // Connect AI tool changes to tray tooltip
    QObject::connect(settingsDialog, &SettingsDialog::aiToolChanged,
                     trayIcon, &TrayIcon::onAiToolChanged);

    return app.exec();
}
