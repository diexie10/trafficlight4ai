#include "TrayIcon.h"
#include "FloatingWindow.h"
#include "SettingsDialog.h"
#include <QMenu>
#include <QApplication>
#include <QPainter>
#include <QPixmap>

TrayIcon::TrayIcon(FloatingWindow *window, SettingsDialog *settingsDialog, QObject *parent)
    : QSystemTrayIcon(parent), m_window(window)
{
    setIcon(createIcon(QColor(40, 200, 40))); // default green
    setToolTip("Traffic Light for AI");

    auto *menu = new QMenu(nullptr);
    connect(this, &QObject::destroyed, menu, &QObject::deleteLater);
    auto *toggleAction = menu->addAction("显示/隐藏");
    connect(toggleAction, &QAction::triggered, this, [this]() {
        m_window->setVisible(!m_window->isVisible());
    });

    auto *settingsAction = menu->addAction("设置");
    connect(settingsAction, &QAction::triggered, settingsDialog, [settingsDialog]() {
        settingsDialog->show();
        settingsDialog->raise();
        settingsDialog->activateWindow();
    });

    menu->addSeparator();

    auto *quitAction = menu->addAction("退出");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    setContextMenu(menu);

    connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::onActivated);
}

void TrayIcon::onStateChanged(LightState newState)
{
    switch (newState) {
    case LightState::Working:
        setIcon(createIcon(QColor(220, 40, 40)));
        break;
    case LightState::WaitingConfirm:
        setIcon(createIcon(QColor(240, 200, 20)));
        break;
    case LightState::Idle:
        setIcon(createIcon(QColor(40, 200, 40)));
        break;
    }
}

void TrayIcon::onAiToolChanged(const QString &displayName)
{
    setToolTip("Traffic Light for " + displayName);
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        m_window->setVisible(!m_window->isVisible());
    }
}

QIcon TrayIcon::createIcon(const QColor &color) const
{
    const int size = 64;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(4, 4, size - 8, size - 8);

    return QIcon(pixmap);
}
