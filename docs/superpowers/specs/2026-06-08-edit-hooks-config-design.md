# Edit Hooks Config Design

## Goal

Add an "Edit Hooks Config" button next to the existing "View Recommended Hooks Config" button in the Settings dialog. The button opens an editor for the selected AI tool's hooks configuration file, with smart read/write logic that handles different file structures.

## AiToolStrategy Interface Changes

Add two new virtual methods to `AiToolStrategy`:

- `hooksConfigPath()` — returns the absolute path to the tool's hooks config file.
- `hooksIsEntireFile()` — returns `true` if the entire file is the hooks JSON (Codex), `false` if hooks is a nested field (Claude Code, Qoder CN).

| Tool       | `hooksConfigPath()`          | `hooksIsEntireFile()` |
|------------|------------------------------|-----------------------|
| Codex      | `~/.codex/hooks.json`        | `true`                |
| Claude Code| `~/.claude/settings.json`    | `false`               |
| Qoder CN   | `~/.qoder-cn/settings.json`  | `false`               |

## Editor Dialog Behavior

### Read Logic

1. If file exists and `hooksIsEntireFile() == true`: load entire file content into editor.
2. If file exists and `hooksIsEntireFile() == false`: parse JSON, extract `"hooks"` field, format as indented JSON with wrapping `{"hooks": ...}`, display in editor.
3. If file does not exist: pre-fill editor with `hooksTemplate()` output.

### Save Logic

1. Validate JSON before saving. If invalid, show error message and do not save.
2. If `hooksIsEntireFile() == true`: write editor content directly to file.
3. If `hooksIsEntireFile() == false`: read original file (or empty `{}`), parse editor content to extract hooks object, merge into original JSON under `"hooks"` key, write back.
4. Create parent directories if they do not exist.

### UI

- New `QPushButton` "Edit Hooks Config" placed next to the existing "View Recommended Hooks Config" button in a horizontal layout.
- Editor dialog: `QDialog` with `QTextEdit` (read-write), "Save" and "Cancel" buttons.
- Dialog title: "Edit Hooks Config - {tool display name}".
- `QTextEdit` uses monospace font for JSON editing.
- Both buttons are translatable via `tr()`.

## Files to Modify

- `src/AiToolStrategy.h` — add `hooksConfigPath()` and `hooksIsEntireFile()` to interface and all three strategy classes.
- `src/SettingsDialog.h` — add `m_editHooksBtn` member and `onEditHooksConfig()` slot.
- `src/SettingsDialog.cpp` — add button creation, layout, connection, and editor dialog implementation.
- `translations/trafficlight4ai_zh.ts` — add Chinese translations for new strings.
- `translations/trafficlight4ai_ja.ts` — add Japanese translations for new strings.
- `tests/test_ai_tool_strategy.cpp` — add tests for `hooksConfigPath()` and `hooksIsEntireFile()`.

## Testing

- Verify `hooksConfigPath()` returns expected paths for all three tools.
- Verify `hooksIsEntireFile()` returns correct values.
- Manual testing: open editor for each tool, verify pre-fill on missing file, save, re-open to confirm persistence.
