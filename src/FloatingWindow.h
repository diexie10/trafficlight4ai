#pragma once

#include <QWidget>
#include <QPoint>

class TrafficLightWidget;
class ConfigManager;
class SettingsDialog;

class FloatingWindow : public QWidget {
    Q_OBJECT

public:
    explicit FloatingWindow(TrafficLightWidget *lightWidget, ConfigManager *config,
                            QWidget *parent = nullptr);

    void setSettingsDialog(SettingsDialog *dialog);
    void setStayOnTop(bool on);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    ConfigManager *m_config;
    SettingsDialog *m_settingsDialog = nullptr;
    QPoint m_dragStartPos;
    bool m_dragging = false;
};
