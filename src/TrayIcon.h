#pragma once

#include <QSystemTrayIcon>
#include "StateManager.h"

class FloatingWindow;
class SettingsDialog;

class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT

public:
    explicit TrayIcon(FloatingWindow *window, SettingsDialog *settingsDialog,
                      QObject *parent = nullptr);

public slots:
    void onStateChanged(LightState newState);
    void onAiToolChanged(const QString &displayName);
    void onActiveAlphaChanged(qreal alpha);

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QIcon createIcon(const QColor &color) const;

    FloatingWindow *m_window;
    LightState m_state = LightState::Idle;
    QColor m_currentColor{40, 200, 40};
};
