#include "SettingsDialog.h"
#include "ConfigManager.h"
#include "TrafficLightWidget.h"
#include "IpcServer.h"
#include "StateManager.h"
#include "AiToolStrategy.h"
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QClipboard>
#include <QApplication>

SettingsDialog::SettingsDialog(ConfigManager *config, TrafficLightWidget *lightWidget,
                               IpcServer *ipcServer, StateManager *stateManager,
                               QWidget *parent)
    : QDialog(parent), m_config(config), m_lightWidget(lightWidget),
      m_ipcServer(ipcServer), m_stateManager(stateManager)
{
    setWindowTitle("设置 - Traffic Light for AI");
    setMinimumSize(400, 320);

    // AI tool
    m_aiToolCombo = new QComboBox();
    for (auto *s : AiToolRegistry::strategies())
        m_aiToolCombo->addItem(s->displayName(), s->id());

    // Timeout
    m_timeoutSpin = new QSpinBox();
    m_timeoutSpin->setRange(0, 3600);
    m_timeoutSpin->setSingleStep(30);
    m_timeoutSpin->setSuffix(" 秒");
    m_timeoutSpin->setSpecialValueText("禁用");

    // Window size
    m_sizeCombo = new QComboBox();
    m_sizeCombo->addItems({"小", "中", "大"});

    // Animation mode
    m_modeCombo = new QComboBox();
    m_modeCombo->addItems({"呼吸灯", "经典闪烁"});

    // Animation period
    m_periodSlider = new QSlider(Qt::Horizontal);
    m_periodSlider->setRange(200, 5000);
    m_periodSlider->setSingleStep(100);
    m_periodSpin = new QSpinBox();
    m_periodSpin->setRange(200, 5000);
    m_periodSpin->setSingleStep(100);
    m_periodSpin->setSuffix(" ms");

    auto *periodLayout = new QHBoxLayout();
    periodLayout->addWidget(m_periodSlider);
    periodLayout->addWidget(m_periodSpin);

    // Socket path
    m_socketEdit = new QLineEdit();

    // Form layout
    auto *form = new QFormLayout();
    form->addRow("AI 工具:", m_aiToolCombo);
    form->addRow("超时时间:", m_timeoutSpin);
    form->addRow("窗口大小:", m_sizeCombo);
    form->addRow("动画模式:", m_modeCombo);
    form->addRow("动画周期:", periodLayout);
    form->addRow("Socket 路径:", m_socketEdit);

    // Hooks template button
    auto *hooksBtn = new QPushButton("查看推荐 Hooks 配置");

    // Buttons
    auto *okBtn = new QPushButton("确定");
    auto *cancelBtn = new QPushButton("取消");
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(hooksBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addLayout(btnLayout);

    // Connections
    connect(m_aiToolCombo, &QComboBox::currentIndexChanged,
            this, &SettingsDialog::onAiToolChanged);
    connect(m_timeoutSpin, &QSpinBox::valueChanged,
            this, &SettingsDialog::onTimeoutChanged);
    connect(m_sizeCombo, &QComboBox::currentIndexChanged,
            this, &SettingsDialog::onWindowSizeChanged);
    connect(m_modeCombo, &QComboBox::currentIndexChanged,
            this, &SettingsDialog::onAnimationModeChanged);
    connect(m_periodSlider, &QSlider::valueChanged,
            this, &SettingsDialog::onAnimationPeriodChanged);
    connect(m_periodSpin, &QSpinBox::valueChanged,
            this, &SettingsDialog::onAnimationPeriodChanged);
    connect(m_socketEdit, &QLineEdit::editingFinished,
            this, &SettingsDialog::onSocketPathEdited);
    connect(hooksBtn, &QPushButton::clicked,
            this, &SettingsDialog::onShowHooksTemplate);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancel);
}

void SettingsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    takeSnapshot();

    // AI tool
    const QString toolId = m_config->aiTool();
    for (int i = 0; i < m_aiToolCombo->count(); ++i) {
        if (m_aiToolCombo->itemData(i).toString() == toolId) {
            m_aiToolCombo->blockSignals(true);
            m_aiToolCombo->setCurrentIndex(i);
            m_aiToolCombo->blockSignals(false);
            break;
        }
    }

    // Timeout
    m_timeoutSpin->blockSignals(true);
    m_timeoutSpin->setValue(m_config->timeoutSec());
    m_timeoutSpin->blockSignals(false);

    // Window size
    const QStringList sizes = {"small", "medium", "large"};
    m_sizeCombo->blockSignals(true);
    m_sizeCombo->setCurrentIndex(sizes.indexOf(m_config->windowSize()));
    m_sizeCombo->blockSignals(false);

    // Animation mode
    const QStringList modes = {"breathing", "classic"};
    m_modeCombo->blockSignals(true);
    m_modeCombo->setCurrentIndex(modes.indexOf(m_config->animationMode()));
    m_modeCombo->blockSignals(false);

    // Animation period
    m_periodSlider->blockSignals(true);
    m_periodSlider->setValue(m_config->animationPeriodMs());
    m_periodSlider->blockSignals(false);
    m_periodSpin->blockSignals(true);
    m_periodSpin->setValue(m_config->animationPeriodMs());
    m_periodSpin->blockSignals(false);

    // Socket path — disable editing when overridden by env var
    const bool envOverride = !qgetenv("TL4AI_SOCKET").isEmpty();
    m_socketEdit->setEnabled(!envOverride);
    m_socketEdit->setText(m_config->socketPath());
    if (envOverride)
        m_socketEdit->setPlaceholderText("由 TL4AI_SOCKET 环境变量控制");
}

void SettingsDialog::takeSnapshot()
{
    m_snapAiTool = m_config->aiTool();
    m_snapTimeoutSec = m_config->timeoutSec();
    m_snapSize = m_config->windowSize();
    m_snapMode = m_config->animationMode();
    m_snapPeriodMs = m_config->animationPeriodMs();
    m_snapSocketPath = m_config->socketPath();
}

void SettingsDialog::restoreSnapshot()
{
    // Restore AI tool
    m_config->setAiTool(m_snapAiTool);
    auto *strategy = AiToolRegistry::find(m_snapAiTool);
    emit aiToolChanged(strategy->displayName());

    // Restore timeout
    m_config->setTimeoutSec(m_snapTimeoutSec);
    m_stateManager->setTimeoutSec(m_snapTimeoutSec);

    // Restore window size
    m_config->setWindowSize(m_snapSize);
    const QStringList sizes = {"small", "medium", "large"};
    int idx = sizes.indexOf(m_snapSize);
    TrafficLightWidget::SizePreset presets[] = {
        TrafficLightWidget::Small, TrafficLightWidget::Medium, TrafficLightWidget::Large
    };
    m_lightWidget->setSizePreset(presets[idx >= 0 ? idx : 0]);

    // Restore animation
    m_lightWidget->setAnimationMode(m_snapMode);
    m_config->setAnimationMode(m_snapMode);
    m_lightWidget->setAnimationPeriodMs(m_snapPeriodMs);
    m_config->setAnimationPeriodMs(m_snapPeriodMs);

    // Restore socket path
    if (m_config->socketPath() != m_snapSocketPath) {
        m_config->setSocketPath(m_snapSocketPath);
        m_ipcServer->restart(m_snapSocketPath);
    }
}

void SettingsDialog::onAiToolChanged(int index)
{
    const QString toolId = m_aiToolCombo->itemData(index).toString();
    m_config->setAiTool(toolId);
    auto *strategy = AiToolRegistry::find(toolId);
    emit aiToolChanged(strategy->displayName());
}

void SettingsDialog::onTimeoutChanged(int value)
{
    m_config->setTimeoutSec(value);
    m_stateManager->setTimeoutSec(value);
}

void SettingsDialog::onWindowSizeChanged(int index)
{
    const QStringList sizes = {"small", "medium", "large"};
    TrafficLightWidget::SizePreset presets[] = {
        TrafficLightWidget::Small, TrafficLightWidget::Medium, TrafficLightWidget::Large
    };
    m_config->setWindowSize(sizes.at(index));
    m_lightWidget->setSizePreset(presets[index]);
}

void SettingsDialog::onAnimationModeChanged(int index)
{
    const QStringList modes = {"breathing", "classic"};
    m_config->setAnimationMode(modes.at(index));
    m_lightWidget->setAnimationMode(modes.at(index));
}

void SettingsDialog::onAnimationPeriodChanged(int value)
{
    m_periodSlider->blockSignals(true);
    m_periodSlider->setValue(value);
    m_periodSlider->blockSignals(false);
    m_periodSpin->blockSignals(true);
    m_periodSpin->setValue(value);
    m_periodSpin->blockSignals(false);

    m_config->setAnimationPeriodMs(value);
    m_lightWidget->setAnimationPeriodMs(value);
}

void SettingsDialog::onSocketPathEdited()
{
    const QString newPath = m_socketEdit->text().trimmed();
    if (newPath.isEmpty() || newPath == m_config->socketPath())
        return;
    m_config->setSocketPath(newPath);
    m_ipcServer->restart(newPath);
}

void SettingsDialog::onShowHooksTemplate()
{
    const QString toolId = m_aiToolCombo->currentData().toString();
    auto *strategy = AiToolRegistry::find(toolId);

    auto *dlg = new QDialog(this);
    dlg->setWindowTitle("推荐 Hooks 配置 - " + strategy->displayName());
    dlg->setMinimumSize(450, 350);

    auto *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setPlainText(strategy->hooksTemplate());

    auto *copyBtn = new QPushButton("复制");
    auto *closeBtn = new QPushButton("关闭");

    connect(copyBtn, &QPushButton::clicked, this, [textEdit]() {
        QApplication::clipboard()->setText(textEdit->toPlainText());
    });
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(copyBtn);
    btnLayout->addWidget(closeBtn);

    auto *layout = new QVBoxLayout(dlg);
    layout->addWidget(textEdit);
    layout->addLayout(btnLayout);

    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void SettingsDialog::reject()
{
    restoreSnapshot();
    QDialog::reject();
}

void SettingsDialog::onCancel()
{
    reject();
}
