# Code Review Fixes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix all 17 issues identified in the full-codebase code review — security, bugs, missing includes, efficiency, and test coverage gaps.

**Architecture:** Fixes are grouped into 6 tasks by concern area. Each task is independently committable. No new files are created except test additions.

**Tech Stack:** C++17, Qt 6, CMake, QTest

---

### Task 1: Security — IPC socket access control and read size limit

**Files:**
- Modify: `src/IpcServer.cpp:59-70` (constructor), `src/IpcServer.cpp:84-110` (restart), `src/IpcServer.cpp:117-138` (onNewConnection)

- [ ] **Step 1: Add UserAccessOption before listen() in constructor**

In `src/IpcServer.cpp`, change the constructor to set socket options before listen:

```cpp
IpcServer::IpcServer(StateManager *stateManager, const QString &socketPath, QObject *parent)
    : QObject(parent), m_server(std::make_unique<QLocalServer>()),
      m_stateManager(stateManager), m_socketPath(socketPath)
{
    removeStaleSocket(m_socketPath);

    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    connectServer();
    m_ownsSocket = m_server->listen(m_socketPath);
    if (!m_ownsSocket)
        qWarning("IpcServer: failed to listen on %s: %s",
                 qPrintable(m_socketPath), qPrintable(m_server->errorString()));
}
```

- [ ] **Step 2: Add UserAccessOption before listen() in restart()**

In `src/IpcServer.cpp` `restart()`, add socket options on newServer before listen:

```cpp
auto newServer = std::make_unique<QLocalServer>();
newServer->setSocketOptions(QLocalServer::UserAccessOption);

removeStaleSocket(newPath);
if (!newServer->listen(newPath)) {
```

- [ ] **Step 3: Replace readAll() with read(64) in onNewConnection**

In `src/IpcServer.cpp` `onNewConnection()`, change the lambda:

```cpp
auto processData = [this, client]() {
    QByteArray data = client->read(64);
    if (!data.isEmpty())
        m_stateManager->handleCommand(QString::fromUtf8(data));
    client->disconnectFromServer();
    client->deleteLater();
};
```

- [ ] **Step 4: Commit**

```bash
git add src/IpcServer.cpp
git commit -m "fix: restrict IPC socket to owner-only access and cap read size"
```

---

### Task 2: Bugs — IPC double-delete, restart race, missing "small" size, SoundUtils leak

**Files:**
- Modify: `src/IpcServer.cpp:117-138` (onNewConnection), `src/IpcServer.cpp:84-110` (restart)
- Modify: `src/main.cpp:62-70`
- Modify: `src/SoundUtils.cpp:9-42`

- [ ] **Step 1: Fix double-delete in onNewConnection — move disconnected signal into else branch**

In `src/IpcServer.cpp`, restructure `onNewConnection()`:

```cpp
void IpcServer::onNewConnection()
{
    while (QLocalSocket *client = m_server->nextPendingConnection()) {
        auto processData = [this, client]() {
            QByteArray data = client->read(64);
            if (!data.isEmpty())
                m_stateManager->handleCommand(QString::fromUtf8(data));
            client->disconnectFromServer();
            client->deleteLater();
        };

        if (client->bytesAvailable()) {
            processData();
        } else {
            connect(client, &QLocalSocket::disconnected, client, &QObject::deleteLater);
            connect(client, &QLocalSocket::readyRead, this, processData);
        }
    }
}
```

- [ ] **Step 2: Fix restart() race — connect newConnection before listen()**

In `src/IpcServer.cpp`, restructure `restart()`:

```cpp
bool IpcServer::restart(const QString &newPath)
{
    if (newPath == m_socketPath)
        return isListening();

    auto newServer = std::make_unique<QLocalServer>();
    newServer->setSocketOptions(QLocalServer::UserAccessOption);

    removeStaleSocket(newPath);
    if (!newServer->listen(newPath)) {
        qWarning("IpcServer: failed to listen on %s: %s",
                 qPrintable(newPath), qPrintable(newServer->errorString()));
        return false;
    }

    const QString oldPath = m_socketPath;
    const bool oldOwned = m_ownsSocket;

    m_server->close();
    if (oldOwned)
        removeOwnedServer(oldPath);

    m_server = std::move(newServer);
    m_socketPath = newPath;
    m_ownsSocket = true;
    connectServer();
    return true;
}
```

Note: `connectServer()` is called after `m_server` is swapped, which connects `newConnection` on the new server. Since `listen()` was already called, any connection arriving between `listen()` and `connectServer()` would be queued in `nextPendingConnection`. As soon as `connectServer()` fires, `onNewConnection` will drain the queue. This is safe because the signal is queued and no event loop runs between `listen()` and `connectServer()` in the same function.

- [ ] **Step 3: Add missing "small" case in main.cpp**

In `src/main.cpp`, add the missing branch after line 64:

```cpp
const QString size = config.windowSize();
if (size == "xsmall")
    lightWidget->setSizePreset(TrafficLightWidget::ExtraSmall);
else if (size == "small")
    lightWidget->setSizePreset(TrafficLightWidget::Small);
else if (size == "medium")
    lightWidget->setSizePreset(TrafficLightWidget::Medium);
else if (size == "large")
    lightWidget->setSizePreset(TrafficLightWidget::Large);
else if (size == "xlarge")
    lightWidget->setSizePreset(TrafficLightWidget::ExtraLarge);
```

- [ ] **Step 4: Fix SoundUtils leak — parent audioOutput to player**

In `src/SoundUtils.cpp`, parent `audioOutput` to `player` and remove manual `audioOutput->deleteLater()` calls:

```cpp
void playSound(const QString &filePath, QObject *errorContext)
{
    if (!filePath.isEmpty() && QFile::exists(filePath)) {
        auto *player = new QMediaPlayer();
        auto *audioOutput = new QAudioOutput(player);
        player->setAudioOutput(audioOutput);
        player->setSource(QUrl::fromLocalFile(filePath));

        QObject::connect(player, &QMediaPlayer::errorOccurred,
                         player, [player, errorContext, filePath]
                         (QMediaPlayer::Error, const QString &) {
            if (errorContext) {
                auto *widget = qobject_cast<QWidget *>(errorContext);
                QMessageBox::warning(widget,
                    QObject::tr("Audio Error"),
                    QObject::tr("Invalid audio file: %1").arg(filePath));
            }
            player->deleteLater();
        });

        QObject::connect(player, &QMediaPlayer::playbackStateChanged,
                         player, [player](QMediaPlayer::PlaybackState state) {
            if (state == QMediaPlayer::StoppedState) {
                player->deleteLater();
            }
        });

        player->play();
    } else {
        QApplication::beep();
    }
}
```

- [ ] **Step 5: Commit**

```bash
git add src/IpcServer.cpp src/main.cpp src/SoundUtils.cpp
git commit -m "fix: IPC double-delete, restart race, missing small size, SoundUtils leak"
```

---

### Task 3: Missing explicit #includes (CLAUDE.md compliance)

**Files:**
- Modify: `src/TrayIcon.h`
- Modify: `src/IpcServer.h`
- Modify: `src/TrafficLightWidget.cpp`
- Modify: `tests/test_ai_tool_strategy.cpp`
- Modify: `tests/test_tl4ai_ctl.cpp`

- [ ] **Step 1: Add QColor and QIcon to TrayIcon.h**

Add after `#include <QSystemTrayIcon>`:

```cpp
#include <QColor>
#include <QIcon>
```

- [ ] **Step 2: Add QString to IpcServer.h**

Add after `#include <QObject>`:

```cpp
#include <QString>
```

- [ ] **Step 3: Add QEasingCurve to TrafficLightWidget.cpp**

Add after `#include <QPainter>`:

```cpp
#include <QEasingCurve>
```

- [ ] **Step 4: Add QRegularExpression to test_ai_tool_strategy.cpp**

Add after `#include <QtTest>`:

```cpp
#include <QRegularExpression>
```

- [ ] **Step 5: Add QTemporaryDir and QFile to test_tl4ai_ctl.cpp**

Add after `#include <QProcess>`:

```cpp
#include <QTemporaryDir>
#include <QFile>
```

- [ ] **Step 6: Commit**

```bash
git add src/TrayIcon.h src/IpcServer.h src/TrafficLightWidget.cpp tests/test_ai_tool_strategy.cpp tests/test_tl4ai_ctl.cpp
git commit -m "fix: add missing explicit Qt includes per CLAUDE.md rules"
```

---

### Task 4: Efficiency — TrayIcon pixmap reuse and ConfigManager batch save

**Files:**
- Modify: `src/TrayIcon.h`
- Modify: `src/TrayIcon.cpp`
- Modify: `src/ConfigManager.h`
- Modify: `src/ConfigManager.cpp`

- [ ] **Step 1: Reuse QPixmap in TrayIcon instead of allocating per frame**

In `src/TrayIcon.h`, add a member:

```cpp
QPixmap m_iconPixmap{64, 64};
```

In `src/TrayIcon.cpp`, change `createIcon()` to reuse the pixmap:

```cpp
QIcon TrayIcon::createIcon(const QColor &color) const
{
    m_iconPixmap.fill(Qt::transparent);

    QPainter painter(&m_iconPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(4, 4, 64 - 8, 64 - 8);

    return QIcon(m_iconPixmap);
}
```

Since `m_iconPixmap` is mutable state used in a `const` method, declare it as `mutable`:

```cpp
mutable QPixmap m_iconPixmap{64, 64};
```

- [ ] **Step 2: Add beginBatchSave/endBatchSave to ConfigManager**

In `src/ConfigManager.h`, add:

```cpp
void beginBatchSave();
void endBatchSave();
```

And a private member:

```cpp
bool m_batchSave = false;
```

In `src/ConfigManager.cpp`, implement:

```cpp
void ConfigManager::beginBatchSave()
{
    m_batchSave = true;
}

void ConfigManager::endBatchSave()
{
    m_batchSave = false;
    save();
}
```

And guard `save()` calls in each setter. Change the existing `save()` method body to:

At the end of every setter (the private `save()` calls), wrap them:

Actually, a cleaner approach — modify `save()` itself to check the flag:

```cpp
void ConfigManager::save()
{
    if (m_batchSave)
        return;

    QDir dir = QFileInfo(m_configPath).absoluteDir();
    if (!dir.exists())
        dir.mkpath(".");

    QFile file(m_configPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(m_root).toJson(QJsonDocument::Indented));
        file.close();
    }
}
```

- [ ] **Step 3: Use batch save in SettingsDialog::restoreSnapshot()**

In `src/SettingsDialog.cpp`, wrap `restoreSnapshot()` setter calls:

```cpp
void SettingsDialog::restoreSnapshot()
{
    m_config->beginBatchSave();

    // ... all existing restore code unchanged ...

    m_config->endBatchSave();
}
```

- [ ] **Step 4: Commit**

```bash
git add src/TrayIcon.h src/TrayIcon.cpp src/ConfigManager.h src/ConfigManager.cpp src/SettingsDialog.cpp
git commit -m "perf: reuse TrayIcon pixmap and batch ConfigManager saves"
```

---

### Task 5: Test coverage — ConfigManager language/sound, Claude Code events, StateManager setTimeoutSec(0) while running

**Files:**
- Modify: `tests/test_config_manager.cpp`
- Modify: `tests/test_ai_tool_strategy.cpp`
- Modify: `tests/test_state_manager.cpp`

- [ ] **Step 1: Add language and sound tests to test_config_manager.cpp**

Add these test slots before the closing `};`:

```cpp
void defaultLanguage()
{
    ConfigManager cm(m_configPath);
    QCOMPARE(cm.language(), QString("en"));
}

void setAndGetLanguage()
{
    ConfigManager cm(m_configPath);
    cm.setLanguage("zh");
    QCOMPARE(cm.language(), QString("zh"));
}

void languagePersists()
{
    {
        ConfigManager cm(m_configPath);
        cm.setLanguage("ja");
    }
    ConfigManager cm2(m_configPath);
    QCOMPARE(cm2.language(), QString("ja"));
}

void defaultYellowSoundEnabled()
{
    ConfigManager cm(m_configPath);
    QCOMPARE(cm.yellowSoundEnabled(), true);
}

void setAndGetYellowSoundEnabled()
{
    ConfigManager cm(m_configPath);
    cm.setYellowSoundEnabled(false);
    QCOMPARE(cm.yellowSoundEnabled(), false);
}

void defaultGreenSoundEnabled()
{
    ConfigManager cm(m_configPath);
    QCOMPARE(cm.greenSoundEnabled(), true);
}

void setAndGetGreenSoundEnabled()
{
    ConfigManager cm(m_configPath);
    cm.setGreenSoundEnabled(false);
    QCOMPARE(cm.greenSoundEnabled(), false);
}

void defaultYellowSoundFile()
{
    ConfigManager cm(m_configPath);
    QCOMPARE(cm.yellowSoundFile(), QString());
}

void setAndGetYellowSoundFile()
{
    ConfigManager cm(m_configPath);
    cm.setYellowSoundFile("/tmp/alert.wav");
    QCOMPARE(cm.yellowSoundFile(), QString("/tmp/alert.wav"));
}

void defaultGreenSoundFile()
{
    ConfigManager cm(m_configPath);
    QCOMPARE(cm.greenSoundFile(), QString());
}

void setAndGetGreenSoundFile()
{
    ConfigManager cm(m_configPath);
    cm.setGreenSoundFile("/tmp/done.ogg");
    QCOMPARE(cm.greenSoundFile(), QString("/tmp/done.ogg"));
}

void soundSettingsPersist()
{
    {
        ConfigManager cm(m_configPath);
        cm.setYellowSoundEnabled(false);
        cm.setYellowSoundFile("/tmp/y.wav");
        cm.setGreenSoundEnabled(false);
        cm.setGreenSoundFile("/tmp/g.mp3");
    }
    ConfigManager cm2(m_configPath);
    QCOMPARE(cm2.yellowSoundEnabled(), false);
    QCOMPARE(cm2.yellowSoundFile(), QString("/tmp/y.wav"));
    QCOMPARE(cm2.greenSoundEnabled(), false);
    QCOMPARE(cm2.greenSoundFile(), QString("/tmp/g.mp3"));
}
```

- [ ] **Step 2: Add Claude Code event validation test to test_ai_tool_strategy.cpp**

Add this test slot:

```cpp
void claudeTemplateOnlyUsesValidEvents()
{
    const QStringList claudeEvents = {
        "PreToolUse", "PostToolUse", "Notification", "Stop",
        "SubagentStart", "SubagentStop", "UserPromptSubmit",
        "PermissionRequest", "SessionEnd"
    };

    ClaudeCodeStrategy claude;
    const QString tmpl = claude.hooksTemplate();
    QRegularExpression re(R"RE("(\w+)":\s*[\[{])RE");
    auto it = re.globalMatch(tmpl);
    while (it.hasNext()) {
        auto match = it.next();
        QString event = match.captured(1);
        if (event == "hooks" || event == "command")
            continue;
        QVERIFY2(claudeEvents.contains(event),
                  qPrintable("Invalid Claude Code event: " + event));
    }
}
```

- [ ] **Step 3: Add setTimeoutSec(0) while running test to test_state_manager.cpp**

Add this test slot:

```cpp
void setTimeoutZeroWhileWorkingCancelsTimer()
{
    StateManager sm;
    sm.setTimeoutSec(1);
    sm.setState(LightState::Working);
    // Disable timeout while working
    sm.setTimeoutSec(0);
    QTest::qWait(1500);
    QCOMPARE(sm.state(), LightState::Working); // timer was cancelled
}
```

- [ ] **Step 4: Commit**

```bash
git add tests/test_config_manager.cpp tests/test_ai_tool_strategy.cpp tests/test_state_manager.cpp
git commit -m "test: add coverage for language, sound, Claude Code events, and timeout cancel"
```

---

### Task 6: Deduplicate sizes/presets mapping

**Files:**
- Modify: `src/TrafficLightWidget.h`
- Modify: `src/TrafficLightWidget.cpp`
- Modify: `src/main.cpp`
- Modify: `src/SettingsDialog.cpp`

- [ ] **Step 1: Add static sizePresetFromString() to TrafficLightWidget**

In `src/TrafficLightWidget.h`, add a public static method:

```cpp
static SizePreset sizePresetFromString(const QString &size);
```

In `src/TrafficLightWidget.cpp`, implement:

```cpp
TrafficLightWidget::SizePreset TrafficLightWidget::sizePresetFromString(const QString &size)
{
    if (size == "xsmall") return ExtraSmall;
    if (size == "small")  return Small;
    if (size == "medium") return Medium;
    if (size == "large")  return Large;
    if (size == "xlarge") return ExtraLarge;
    return Small; // default
}
```

- [ ] **Step 2: Use sizePresetFromString in main.cpp**

Replace the if-else chain in `src/main.cpp`:

```cpp
lightWidget->setSizePreset(TrafficLightWidget::sizePresetFromString(config.windowSize()));
```

- [ ] **Step 3: Use sizePresetFromString in SettingsDialog::onWindowSizeChanged**

In `src/SettingsDialog.cpp`, simplify `onWindowSizeChanged()`:

```cpp
void SettingsDialog::onWindowSizeChanged(int index)
{
    const QStringList sizes = {"xsmall", "small", "medium", "large", "xlarge"};
    if (index < 0 || index >= sizes.size())
        return;

    QWidget *window = m_lightWidget->window();
    QPoint pos = window->pos();

    m_config->setWindowSize(sizes.at(index));
    m_lightWidget->setSizePreset(TrafficLightWidget::sizePresetFromString(sizes.at(index)));

    resizeFloatingWindowAt(pos, true);
}
```

- [ ] **Step 4: Use sizePresetFromString in SettingsDialog::restoreSnapshot**

In `src/SettingsDialog.cpp`, simplify the size restore block in `restoreSnapshot()`:

```cpp
// Restore window size (preserve position)
QWidget *window = m_lightWidget->window();
QPoint pos = window->pos();
m_config->setWindowSize(m_snapSize);
m_lightWidget->setSizePreset(TrafficLightWidget::sizePresetFromString(m_snapSize));
resizeFloatingWindowAt(pos, false);
```

- [ ] **Step 5: Commit**

```bash
git add src/TrafficLightWidget.h src/TrafficLightWidget.cpp src/main.cpp src/SettingsDialog.cpp
git commit -m "refactor: deduplicate size preset mapping with sizePresetFromString()"
```
