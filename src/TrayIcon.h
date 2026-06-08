#pragma once

#include <QSystemTrayIcon>
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include "StateManager.h"

class QAction;
class FloatingWindow;
class SettingsDialog;

class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT

public:
    explicit TrayIcon(FloatingWindow *window, SettingsDialog *settingsDialog,
                      QObject *parent = nullptr);

    void retranslateUi();

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
    QAction *m_toggleAction;
    QAction *m_settingsAction;
    QAction *m_quitAction;
    QString m_aiToolName;
    mutable QPixmap m_iconPixmap{64, 64};
};
