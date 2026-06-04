#pragma once

#include <QDialog>

class QComboBox;
class QSlider;
class QSpinBox;
class QLineEdit;
class ConfigManager;
class TrafficLightWidget;
class IpcServer;
class StateManager;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(ConfigManager *config, TrafficLightWidget *lightWidget,
                            IpcServer *ipcServer, StateManager *stateManager,
                            QWidget *parent = nullptr);

signals:
    void aiToolChanged(const QString &displayName);

public slots:
    void reject() override;

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onAiToolChanged(int index);
    void onTimeoutChanged(int value);
    void onWindowSizeChanged(int index);
    void onAnimationModeChanged(int index);
    void onAnimationPeriodChanged(int value);
    void onSocketPathEdited();
    void onShowHooksTemplate();
    void onCancel();

private:
    void takeSnapshot();
    void restoreSnapshot();

    ConfigManager *m_config;
    TrafficLightWidget *m_lightWidget;
    IpcServer *m_ipcServer;
    StateManager *m_stateManager;

    QComboBox *m_aiToolCombo;
    QSpinBox *m_timeoutSpin;
    QComboBox *m_sizeCombo;
    QComboBox *m_modeCombo;
    QSlider *m_periodSlider;
    QSpinBox *m_periodSpin;
    QLineEdit *m_socketEdit;

    // Snapshot for cancel
    QString m_snapAiTool;
    int m_snapTimeoutSec;
    QString m_snapSize;
    QString m_snapMode;
    int m_snapPeriodMs;
    QString m_snapSocketPath;
};
