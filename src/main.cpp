#include <QApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QSoundEffect>
#include <QUrl>
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

static void playSound(const QString &filePath)
{
    if (!filePath.isEmpty() && QFile::exists(filePath)) {
        auto *effect = new QSoundEffect();
        effect->setSource(QUrl::fromLocalFile(filePath));
        effect->play();
        QObject::connect(effect, &QSoundEffect::playingChanged, effect, [effect]() {
            if (!effect->isPlaying())
                effect->deleteLater();
        });
    } else {
        QApplication::beep();
    }
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

    // Connect state changes to UI (trayIcon first so m_state is updated
    // before lightWidget's animation stop triggers activeAlphaChanged)
    QObject::connect(&stateManager, &StateManager::stateChanged,
                     trayIcon, &TrayIcon::onStateChanged);
    QObject::connect(&stateManager, &StateManager::stateChanged,
                     lightWidget, &TrafficLightWidget::onStateChanged);

    // Sound notifications on state change
    QObject::connect(&stateManager, &StateManager::stateChanged,
                     [&config](LightState state) {
        if (state == LightState::WaitingConfirm && config.yellowSoundEnabled())
            playSound(config.yellowSoundFile());
        else if (state == LightState::Idle && config.greenSoundEnabled())
            playSound(config.greenSoundFile());
    });

    // Connect animation alpha to tray icon blinking
    QObject::connect(lightWidget, &TrafficLightWidget::activeAlphaChanged,
                     trayIcon, &TrayIcon::onActiveAlphaChanged);

    // Connect AI tool changes to tray tooltip
    QObject::connect(settingsDialog, &SettingsDialog::aiToolChanged,
                     trayIcon, &TrayIcon::onAiToolChanged);

    return app.exec();
}
