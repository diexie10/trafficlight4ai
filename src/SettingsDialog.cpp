#include "SettingsDialog.h"
#include "ConfigManager.h"
#include "TrafficLightWidget.h"
#include "IpcServer.h"
#include "StateManager.h"
#include "AiToolStrategy.h"
#include "SoundUtils.h"
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QClipboard>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
#include <QApplication>

SettingsDialog::SettingsDialog(ConfigManager *config, TrafficLightWidget *lightWidget,
                               IpcServer *ipcServer, StateManager *stateManager,
                               QWidget *parent)
    : QDialog(parent), m_config(config), m_lightWidget(lightWidget),
      m_ipcServer(ipcServer), m_stateManager(stateManager)
{
    setMinimumSize(400, 460);

    // Language
    m_langCombo = new QComboBox();
    m_langCombo->addItem("English", "en");
    m_langCombo->addItem(QString::fromUtf8("中文"), "zh");
    m_langCombo->addItem(QString::fromUtf8("日本語"), "ja");

    // AI tool
    m_aiToolCombo = new QComboBox();
    for (auto *s : AiToolRegistry::strategies())
        m_aiToolCombo->addItem(s->displayName(), s->id());

    // Timeout
    m_timeoutSpin = new QSpinBox();
    m_timeoutSpin->setRange(0, 3600);
    m_timeoutSpin->setSingleStep(30);

    // Window size
    m_sizeCombo = new QComboBox();

    // Animation mode
    m_modeCombo = new QComboBox();

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

    // Yellow sound
    m_yellowSoundCheck = new QCheckBox();
    m_yellowSoundEdit = new QLineEdit();
    m_yellowPreviewBtn = new QPushButton();
    m_yellowPreviewBtn->setEnabled(false);
    m_yellowBrowseBtn = new QPushButton();
    auto *yellowSoundLayout = new QHBoxLayout();
    yellowSoundLayout->addWidget(m_yellowSoundCheck);
    yellowSoundLayout->addWidget(m_yellowSoundEdit);
    yellowSoundLayout->addWidget(m_yellowPreviewBtn);
    yellowSoundLayout->addWidget(m_yellowBrowseBtn);

    // Green sound
    m_greenSoundCheck = new QCheckBox();
    m_greenSoundEdit = new QLineEdit();
    m_greenPreviewBtn = new QPushButton();
    m_greenPreviewBtn->setEnabled(false);
    m_greenBrowseBtn = new QPushButton();
    auto *greenSoundLayout = new QHBoxLayout();
    greenSoundLayout->addWidget(m_greenSoundCheck);
    greenSoundLayout->addWidget(m_greenSoundEdit);
    greenSoundLayout->addWidget(m_greenPreviewBtn);
    greenSoundLayout->addWidget(m_greenBrowseBtn);

    // Form layout
    m_formLayout = new QFormLayout();
    m_formLayout->addRow(tr("Language:"), m_langCombo);
    m_formLayout->addRow(tr("AI Tool:"), m_aiToolCombo);
    m_formLayout->addRow(tr("Timeout:"), m_timeoutSpin);
    m_formLayout->addRow(tr("Window Size:"), m_sizeCombo);
    m_formLayout->addRow(tr("Animation Mode:"), m_modeCombo);
    m_formLayout->addRow(tr("Animation Period:"), periodLayout);
    m_formLayout->addRow(tr("Socket Path:"), m_socketEdit);
    m_formLayout->addRow(tr("Yellow Sound:"), yellowSoundLayout);
    m_formLayout->addRow(tr("Green Sound:"), greenSoundLayout);

    // Buttons
    m_hooksBtn = new QPushButton();
    m_okBtn = new QPushButton();
    m_cancelBtn = new QPushButton();
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_hooksBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_okBtn);
    btnLayout->addWidget(m_cancelBtn);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(m_formLayout);
    mainLayout->addLayout(btnLayout);

    // Set all translatable text
    retranslateUi();

    // Connections
    connect(m_langCombo, &QComboBox::currentIndexChanged,
            this, &SettingsDialog::onLanguageChanged);
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
    connect(m_yellowSoundCheck, &QCheckBox::toggled,
            this, &SettingsDialog::onYellowSoundToggled);
    connect(m_greenSoundCheck, &QCheckBox::toggled,
            this, &SettingsDialog::onGreenSoundToggled);
    connect(m_yellowSoundEdit, &QLineEdit::textChanged,
            this, [this]() { updatePreviewButtons(); });
    connect(m_greenSoundEdit, &QLineEdit::textChanged,
            this, [this]() { updatePreviewButtons(); });
    connect(m_yellowPreviewBtn, &QPushButton::clicked,
            this, &SettingsDialog::onPreviewYellowSound);
    connect(m_greenPreviewBtn, &QPushButton::clicked,
            this, &SettingsDialog::onPreviewGreenSound);
    connect(m_yellowBrowseBtn, &QPushButton::clicked,
            this, &SettingsDialog::onBrowseYellowSound);
    connect(m_greenBrowseBtn, &QPushButton::clicked,
            this, &SettingsDialog::onBrowseGreenSound);
    connect(m_hooksBtn, &QPushButton::clicked,
            this, &SettingsDialog::onShowHooksTemplate);
    connect(m_okBtn, &QPushButton::clicked, this, &SettingsDialog::onAccept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancel);
}

void SettingsDialog::retranslateUi()
{
    setWindowTitle(tr("Settings - Traffic Light for AI"));

    // Form labels — safely update each row's label
    const QStringList labels = {
        tr("Language:"), tr("AI Tool:"), tr("Timeout:"), tr("Window Size:"),
        tr("Animation Mode:"), tr("Animation Period:"), tr("Socket Path:"),
        tr("Yellow Sound:"), tr("Green Sound:")
    };
    for (int i = 0; i < labels.size() && i < m_formLayout->rowCount(); ++i) {
        auto *item = m_formLayout->itemAt(i, QFormLayout::LabelRole);
        if (!item) continue;
        auto *label = qobject_cast<QLabel *>(item->widget());
        if (label) label->setText(labels[i]);
    }

    // Timeout suffix and special value
    m_timeoutSpin->setSuffix(" " + tr("sec"));
    m_timeoutSpin->setSpecialValueText(tr("Disabled"));

    // Window size combo
    const int sizeIdx = m_sizeCombo->currentIndex();
    m_sizeCombo->blockSignals(true);
    m_sizeCombo->clear();
    m_sizeCombo->addItems({tr("Small"), tr("Medium"), tr("Large"), tr("Extra Large")});
    if (sizeIdx >= 0) m_sizeCombo->setCurrentIndex(sizeIdx);
    m_sizeCombo->blockSignals(false);

    // Animation mode combo
    const int modeIdx = m_modeCombo->currentIndex();
    m_modeCombo->blockSignals(true);
    m_modeCombo->clear();
    m_modeCombo->addItems({tr("Breathing"), tr("Classic Blink")});
    if (modeIdx >= 0) m_modeCombo->setCurrentIndex(modeIdx);
    m_modeCombo->blockSignals(false);

    // Sound controls
    m_yellowSoundCheck->setText(tr("Enable"));
    m_yellowSoundEdit->setPlaceholderText(tr("Leave empty for system beep"));
    m_yellowPreviewBtn->setText(tr("Preview"));
    m_yellowBrowseBtn->setText(tr("Browse"));

    m_greenSoundCheck->setText(tr("Enable"));
    m_greenSoundEdit->setPlaceholderText(tr("Leave empty for system beep"));
    m_greenPreviewBtn->setText(tr("Preview"));
    m_greenBrowseBtn->setText(tr("Browse"));

    // Buttons
    m_hooksBtn->setText(tr("View Recommended Hooks Config"));
    m_okBtn->setText(tr("OK"));
    m_cancelBtn->setText(tr("Cancel"));

    // Env override placeholder
    if (!qgetenv("TL4AI_SOCKET").isEmpty())
        m_socketEdit->setPlaceholderText(tr("Controlled by TL4AI_SOCKET env var"));
}

void SettingsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    takeSnapshot();

    // Language
    const QString lang = m_config->language();
    for (int i = 0; i < m_langCombo->count(); ++i) {
        if (m_langCombo->itemData(i).toString() == lang) {
            m_langCombo->blockSignals(true);
            m_langCombo->setCurrentIndex(i);
            m_langCombo->blockSignals(false);
            break;
        }
    }

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
    const QStringList sizes = {"small", "medium", "large", "xlarge"};
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

    // Socket path
    const bool envOverride = !qgetenv("TL4AI_SOCKET").isEmpty();
    m_socketEdit->setEnabled(!envOverride);
    m_socketEdit->setText(m_config->socketPath());

    // Sound settings
    m_yellowSoundCheck->blockSignals(true);
    m_yellowSoundCheck->setChecked(m_config->yellowSoundEnabled());
    m_yellowSoundCheck->blockSignals(false);
    m_yellowSoundEdit->setText(m_config->yellowSoundFile());

    m_greenSoundCheck->blockSignals(true);
    m_greenSoundCheck->setChecked(m_config->greenSoundEnabled());
    m_greenSoundCheck->blockSignals(false);
    m_greenSoundEdit->setText(m_config->greenSoundFile());
}

void SettingsDialog::takeSnapshot()
{
    m_snapLang = m_config->language();
    m_snapAiTool = m_config->aiTool();
    m_snapTimeoutSec = m_config->timeoutSec();
    m_snapSize = m_config->windowSize();
    m_snapMode = m_config->animationMode();
    m_snapPeriodMs = m_config->animationPeriodMs();
    m_snapSocketPath = m_config->socketPath();
    m_snapYellowSoundEnabled = m_config->yellowSoundEnabled();
    m_snapYellowSoundFile = m_config->yellowSoundFile();
    m_snapGreenSoundEnabled = m_config->greenSoundEnabled();
    m_snapGreenSoundFile = m_config->greenSoundFile();
}

void SettingsDialog::restoreSnapshot()
{
    // Restore language
    if (m_config->language() != m_snapLang) {
        m_config->setLanguage(m_snapLang);
        emit languageChanged(m_snapLang);
    }

    // Restore AI tool
    m_config->setAiTool(m_snapAiTool);
    if (auto *strategy = AiToolRegistry::find(m_snapAiTool))
        emit aiToolChanged(strategy->displayName());

    // Restore timeout
    m_config->setTimeoutSec(m_snapTimeoutSec);
    m_stateManager->setTimeoutSec(m_snapTimeoutSec);

    // Restore window size (preserve position)
    QWidget *window = m_lightWidget->window();
    QPoint pos = window->pos();
    m_config->setWindowSize(m_snapSize);
    const QStringList sizes = {"small", "medium", "large", "xlarge"};
    int idx = sizes.indexOf(m_snapSize);
    TrafficLightWidget::SizePreset presets[] = {
        TrafficLightWidget::Small, TrafficLightWidget::Medium, TrafficLightWidget::Large, TrafficLightWidget::ExtraLarge
    };
    m_lightWidget->setSizePreset(presets[idx >= 0 ? idx : 0]);
    window->move(pos);
    QTimer::singleShot(0, window, [window, pos]() {
        window->move(pos);
    });

    // Restore animation
    m_lightWidget->setAnimationMode(m_snapMode);
    m_config->setAnimationMode(m_snapMode);
    m_lightWidget->setAnimationPeriodMs(m_snapPeriodMs);
    m_config->setAnimationPeriodMs(m_snapPeriodMs);

    // Restore sound settings
    m_config->setYellowSoundEnabled(m_snapYellowSoundEnabled);
    m_config->setYellowSoundFile(m_snapYellowSoundFile);
    m_config->setGreenSoundEnabled(m_snapGreenSoundEnabled);
    m_config->setGreenSoundFile(m_snapGreenSoundFile);
}

void SettingsDialog::onLanguageChanged(int index)
{
    const QString lang = m_langCombo->itemData(index).toString();
    m_config->setLanguage(lang);
    emit languageChanged(lang);
}

void SettingsDialog::onAiToolChanged(int index)
{
    const QString toolId = m_aiToolCombo->itemData(index).toString();
    m_config->setAiTool(toolId);
    if (auto *strategy = AiToolRegistry::find(toolId))
        emit aiToolChanged(strategy->displayName());
}

void SettingsDialog::onTimeoutChanged(int value)
{
    m_config->setTimeoutSec(value);
    m_stateManager->setTimeoutSec(value);
}

void SettingsDialog::onWindowSizeChanged(int index)
{
    const QStringList sizes = {"small", "medium", "large", "xlarge"};
    TrafficLightWidget::SizePreset presets[] = {
        TrafficLightWidget::Small, TrafficLightWidget::Medium, TrafficLightWidget::Large, TrafficLightWidget::ExtraLarge
    };

    // Preserve window position across size change
    QWidget *window = m_lightWidget->window();
    QPoint pos = window->pos();

    m_config->setWindowSize(sizes.at(index));
    m_lightWidget->setSizePreset(presets[index]);

    // Defer move to after layout recalculation completes
    window->move(pos);
    QTimer::singleShot(0, window, [window, pos, this]() {
        window->move(pos);
        m_config->setWindowPos(pos.x(), pos.y());
    });
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

void SettingsDialog::onYellowSoundToggled(bool checked)
{
    m_config->setYellowSoundEnabled(checked);
}

void SettingsDialog::onGreenSoundToggled(bool checked)
{
    m_config->setGreenSoundEnabled(checked);
}

void SettingsDialog::onPreviewYellowSound()
{
    playSound(m_yellowSoundEdit->text().trimmed(), this);
}

void SettingsDialog::onPreviewGreenSound()
{
    playSound(m_greenSoundEdit->text().trimmed(), this);
}

void SettingsDialog::updatePreviewButtons()
{
    const QString yellowPath = m_yellowSoundEdit->text().trimmed();
    m_yellowPreviewBtn->setEnabled(!yellowPath.isEmpty() && QFile::exists(yellowPath));

    const QString greenPath = m_greenSoundEdit->text().trimmed();
    m_greenPreviewBtn->setEnabled(!greenPath.isEmpty() && QFile::exists(greenPath));
}

void SettingsDialog::onBrowseYellowSound()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select Yellow Sound"), QString(),
                                                 tr("Audio Files (*.wav *.mp3 *.ogg)"));
    if (!file.isEmpty()) {
        m_yellowSoundEdit->setText(file);
        m_config->setYellowSoundFile(file);
    }
}

void SettingsDialog::onBrowseGreenSound()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select Green Sound"), QString(),
                                                 tr("Audio Files (*.wav *.mp3 *.ogg)"));
    if (!file.isEmpty()) {
        m_greenSoundEdit->setText(file);
        m_config->setGreenSoundFile(file);
    }
}

void SettingsDialog::onAccept()
{
    const QString newPath = m_socketEdit->text().trimmed();
    if (!newPath.isEmpty() && newPath != m_config->socketPath()) {
        if (m_ipcServer->restart(newPath)) {
            m_config->setSocketPath(newPath);
        } else {
            m_socketEdit->setText(m_config->socketPath());
            QMessageBox::warning(this, tr("Socket Error"),
                tr("Cannot listen on: %1\nKept original path.").arg(newPath));
            return;
        }
    }

    m_config->setYellowSoundFile(m_yellowSoundEdit->text().trimmed());
    m_config->setGreenSoundFile(m_greenSoundEdit->text().trimmed());

    accept();
}

void SettingsDialog::onShowHooksTemplate()
{
    const QString toolId = m_aiToolCombo->currentData().toString();
    auto *strategy = AiToolRegistry::find(toolId);
    if (!strategy)
        return;

    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Recommended Hooks - %1").arg(strategy->displayName()));
    dlg->setMinimumSize(450, 350);

    auto *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setPlainText(strategy->hooksTemplate());

    auto *copyBtn = new QPushButton(tr("Copy"));
    auto *closeBtn = new QPushButton(tr("Close"));

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
