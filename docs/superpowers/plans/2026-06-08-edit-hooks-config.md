# Edit Hooks Config Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add an "Edit Hooks Config" button to SettingsDialog that opens an editor for the selected AI tool's hooks config file, with smart read/write logic for different file structures.

**Architecture:** Extend `AiToolStrategy` with two new virtual methods (`hooksConfigPath()`, `hooksIsEntireFile()`). Add a new button and slot in `SettingsDialog` that opens an editor dialog with JSON-aware read/write logic. The editor extracts/merges the `"hooks"` field for tools that embed hooks in a larger settings file.

**Tech Stack:** C++17, Qt 6 (Core, Widgets), QJsonDocument

---

### Task 1: Extend AiToolStrategy interface with hooksConfigPath() and hooksIsEntireFile()

**Files:**
- Modify: `src/AiToolStrategy.h`
- Modify: `tests/test_ai_tool_strategy.cpp`

- [ ] **Step 1: Add pure virtual methods to AiToolStrategy and implement in all strategies**

In `src/AiToolStrategy.h`, add two new pure virtual methods to `AiToolStrategy` after `hooksTemplate()`:

```cpp
virtual QString hooksConfigPath() const = 0;
virtual bool hooksIsEntireFile() const = 0;
```

Add `#include <QDir>` at the top (needed for `QDir::homePath()`).

Implement in `CodexStrategy`:

```cpp
QString hooksConfigPath() const override { return QDir::homePath() + "/.codex/hooks.json"; }
bool hooksIsEntireFile() const override { return true; }
```

Implement in `ClaudeCodeStrategy`:

```cpp
QString hooksConfigPath() const override { return QDir::homePath() + "/.claude/settings.json"; }
bool hooksIsEntireFile() const override { return false; }
```

Implement in `QoderCnStrategy`:

```cpp
QString hooksConfigPath() const override { return QDir::homePath() + "/.qoder-cn/settings.json"; }
bool hooksIsEntireFile() const override { return false; }
```

- [ ] **Step 2: Add tests for new methods**

In `tests/test_ai_tool_strategy.cpp`, add these test slots before `registryFindsQoderCn()`:

```cpp
void codexHooksConfigPath()
{
    CodexStrategy codex;
    QVERIFY(codex.hooksConfigPath().endsWith("/.codex/hooks.json"));
}

void codexHooksIsEntireFile()
{
    CodexStrategy codex;
    QCOMPARE(codex.hooksIsEntireFile(), true);
}

void claudeHooksConfigPath()
{
    ClaudeCodeStrategy claude;
    QVERIFY(claude.hooksConfigPath().endsWith("/.claude/settings.json"));
}

void claudeHooksIsNotEntireFile()
{
    ClaudeCodeStrategy claude;
    QCOMPARE(claude.hooksIsEntireFile(), false);
}

void qoderCnHooksConfigPath()
{
    QoderCnStrategy qoderCn;
    QVERIFY(qoderCn.hooksConfigPath().endsWith("/.qoder-cn/settings.json"));
}

void qoderCnHooksIsNotEntireFile()
{
    QoderCnStrategy qoderCn;
    QCOMPARE(qoderCn.hooksIsEntireFile(), false);
}
```

- [ ] **Step 3: Commit**

```bash
git add src/AiToolStrategy.h tests/test_ai_tool_strategy.cpp
git commit -m "feat: add hooksConfigPath() and hooksIsEntireFile() to AiToolStrategy"
```

---

### Task 2: Add "Edit Hooks Config" button and editor dialog to SettingsDialog

**Files:**
- Modify: `src/SettingsDialog.h`
- Modify: `src/SettingsDialog.cpp`

- [ ] **Step 1: Add member and slot declaration in header**

In `src/SettingsDialog.h`, add a new private slot after `onShowHooksTemplate()`:

```cpp
void onEditHooksConfig();
```

Add a new member after `QPushButton *m_hooksBtn;`:

```cpp
QPushButton *m_editHooksBtn;
```

- [ ] **Step 2: Create the button, add to layout, and connect signal**

In `src/SettingsDialog.cpp` constructor, change the buttons section (around line 111-118) from:

```cpp
// Buttons
m_hooksBtn = new QPushButton();
m_okBtn = new QPushButton();
m_cancelBtn = new QPushButton();
auto *btnLayout = new QHBoxLayout();
btnLayout->addWidget(m_hooksBtn);
btnLayout->addStretch();
btnLayout->addWidget(m_okBtn);
btnLayout->addWidget(m_cancelBtn);
```

To:

```cpp
// Buttons
m_hooksBtn = new QPushButton();
m_editHooksBtn = new QPushButton();
m_okBtn = new QPushButton();
m_cancelBtn = new QPushButton();
auto *btnLayout = new QHBoxLayout();
btnLayout->addWidget(m_hooksBtn);
btnLayout->addWidget(m_editHooksBtn);
btnLayout->addStretch();
btnLayout->addWidget(m_okBtn);
btnLayout->addWidget(m_cancelBtn);
```

Add signal connection after the existing `m_hooksBtn` connection (around line 158-159):

```cpp
connect(m_editHooksBtn, &QPushButton::clicked,
        this, &SettingsDialog::onEditHooksConfig);
```

- [ ] **Step 3: Add translatable text for the new button**

In `retranslateUi()`, after `m_hooksBtn->setText(...)` (line 213), add:

```cpp
m_editHooksBtn->setText(tr("Edit Hooks Config"));
```

- [ ] **Step 4: Implement onEditHooksConfig() slot**

Add these includes at the top of `src/SettingsDialog.cpp` if not already present:

```cpp
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFileInfo>
#include <QFont>
```

Add the slot implementation at the end of the file, before `reject()`:

```cpp
void SettingsDialog::onEditHooksConfig()
{
    const QString toolId = m_aiToolCombo->currentData().toString();
    auto *strategy = AiToolRegistry::find(toolId);
    if (!strategy)
        return;

    const QString configPath = strategy->hooksConfigPath();
    const bool entireFile = strategy->hooksIsEntireFile();

    // Read current content
    QString content;
    QFile file(configPath);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray raw = file.readAll();
        file.close();
        if (entireFile) {
            content = QString::fromUtf8(raw);
        } else {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
            if (err.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject hooksObj;
                hooksObj["hooks"] = doc.object()["hooks"];
                content = QJsonDocument(hooksObj).toJson(QJsonDocument::Indented);
            } else {
                content = strategy->hooksTemplate();
            }
        }
    } else {
        content = strategy->hooksTemplate();
    }

    // Build editor dialog
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Edit Hooks Config - %1").arg(strategy->displayName()));
    dlg->setMinimumSize(500, 400);

    auto *textEdit = new QTextEdit();
    textEdit->setPlainText(content);
    QFont monoFont("monospace");
    monoFont.setStyleHint(QFont::Monospace);
    textEdit->setFont(monoFont);

    auto *pathLabel = new QLabel(configPath);
    pathLabel->setWordWrap(true);

    auto *saveBtn = new QPushButton(tr("Save"));
    auto *cancelBtn = new QPushButton(tr("Cancel"));

    connect(saveBtn, &QPushButton::clicked, dlg, [this, dlg, textEdit, configPath, entireFile]() {
        const QString text = textEdit->toPlainText().trimmed();
        QJsonParseError err;
        QJsonDocument editedDoc = QJsonDocument::fromJson(text.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError) {
            QMessageBox::warning(dlg, tr("JSON Error"),
                tr("Invalid JSON at offset %1:\n%2").arg(err.offset).arg(err.errorString()));
            return;
        }

        // Prepare the final content to write
        QByteArray output;
        if (entireFile) {
            output = editedDoc.toJson(QJsonDocument::Indented);
        } else {
            // Read existing file to preserve non-hooks fields
            QJsonObject root;
            QFile existing(configPath);
            if (existing.exists() && existing.open(QIODevice::ReadOnly)) {
                QJsonDocument existingDoc = QJsonDocument::fromJson(existing.readAll());
                existing.close();
                if (existingDoc.isObject())
                    root = existingDoc.object();
            }
            // Merge hooks from edited content
            QJsonObject editedObj = editedDoc.object();
            if (editedObj.contains("hooks"))
                root["hooks"] = editedObj["hooks"];
            else
                root["hooks"] = editedObj;
            output = QJsonDocument(root).toJson(QJsonDocument::Indented);
        }

        // Create parent directory if needed
        QDir dir = QFileInfo(configPath).absoluteDir();
        if (!dir.exists())
            dir.mkpath(".");

        QFile outFile(configPath);
        if (outFile.open(QIODevice::WriteOnly)) {
            outFile.write(output);
            outFile.close();
            dlg->accept();
        } else {
            QMessageBox::warning(dlg, tr("Save Error"),
                tr("Cannot write to: %1").arg(configPath));
        }
    });
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);

    auto *layout = new QVBoxLayout(dlg);
    layout->addWidget(pathLabel);
    layout->addWidget(textEdit);
    layout->addLayout(btnLayout);

    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->exec();
}
```

- [ ] **Step 5: Commit**

```bash
git add src/SettingsDialog.h src/SettingsDialog.cpp
git commit -m "feat: add Edit Hooks Config button and editor dialog"
```

---

### Task 3: Update translations

**Files:**
- Modify: `translations/trafficlight4ai_zh.ts`
- Modify: `translations/trafficlight4ai_ja.ts`

- [ ] **Step 1: Add Chinese translations**

In `translations/trafficlight4ai_zh.ts`, find the `SettingsDialog` context and add entries for:

- `"Edit Hooks Config"` → `"编辑 Hooks 配置"`
- `"Edit Hooks Config - %1"` → `"编辑 Hooks 配置 - %1"`
- `"JSON Error"` → `"JSON 错误"`
- `"Invalid JSON at offset %1:\n%2"` → `"偏移 %1 处 JSON 无效：\n%2"`
- `"Save Error"` → `"保存错误"`
- `"Cannot write to: %1"` → `"无法写入：%1"`

- [ ] **Step 2: Add Japanese translations**

In `translations/trafficlight4ai_ja.ts`, find the `SettingsDialog` context and add entries for:

- `"Edit Hooks Config"` → `"Hooks 設定を編集"`
- `"Edit Hooks Config - %1"` → `"Hooks 設定を編集 - %1"`
- `"JSON Error"` → `"JSON エラー"`
- `"Invalid JSON at offset %1:\n%2"` → `"オフセット %1 で無効な JSON：\n%2"`
- `"Save Error"` → `"保存エラー"`
- `"Cannot write to: %1"` → `"書き込みできません：%1"`

- [ ] **Step 3: Commit**

```bash
git add translations/trafficlight4ai_zh.ts translations/trafficlight4ai_ja.ts
git commit -m "i18n: add translations for Edit Hooks Config feature"
```
