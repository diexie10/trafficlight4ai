#include "FloatingWindow.h"
#include "TrafficLightWidget.h"
#include "ConfigManager.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QApplication>

FloatingWindow::FloatingWindow(TrafficLightWidget *lightWidget, ConfigManager *config,
                               QWidget *parent)
    : QWidget(parent), m_config(config)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    // Apply stay-on-top from config (default true)
    setStayOnTop(m_config->stayOnTop());

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addWidget(lightWidget);

    move(m_config->windowPosX(), m_config->windowPosY());
}

void FloatingWindow::setStayOnTop(bool on)
{
    const bool wasVisible = isVisible();
    if (wasVisible) hide();
    setWindowFlag(Qt::WindowStaysOnTopHint, on);
    if (wasVisible) show();
}

void FloatingWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStartPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void FloatingWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragStartPos);
        event->accept();
    }
}

void FloatingWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        m_config->setWindowPos(x(), y());
        event->accept();
    }
}

void FloatingWindow::setSettingsDialog(SettingsDialog *dialog)
{
    m_settingsDialog = dialog;
}

void FloatingWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    if (m_settingsDialog) {
        auto *settingsAction = menu.addAction(tr("Settings"));
        connect(settingsAction, &QAction::triggered, this, [this]() {
            m_settingsDialog->show();
            m_settingsDialog->raise();
            m_settingsDialog->activateWindow();
        });
    }
    auto *quitAction = menu.addAction(tr("Quit"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    menu.exec(event->globalPos());
}
