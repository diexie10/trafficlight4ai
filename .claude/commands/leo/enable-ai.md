---
description: 在当前项目中初始化 AI 工具配置（外部文档路径、记忆同步、Git 信息等）
argument-hint: [项目名称(可选)]
---

# /enable-ai

在当前项目中执行 AI 工具初始化配置：设置外部文档路径、记忆同步策略、Git 用户信息、交互规则等。

用户传入的参数（如果有）：`$ARGUMENTS`

## 核心原则

- 若有不明白或不明确的地方，一定要先问我。不要自己幻想或无中生有。
- 用户偏好使用中文对话。
- CLAUDE.md 使用中文编写。
- 所有生成的记忆文件（包括 MEMORY.md 索引及各 feedback_*.md / user_*.md 等）使用中文编写。
- 所有生成的文档（包括记忆文件、CLAUDE.md、MEMORY.md 等）编码格式均使用 UTF-8。
- 每一步先检查已有状态，确保幂等（多次执行不破坏已有配置）。
- 每一步完成后简要汇报结果，再进入下一步。

## 执行步骤

按以下顺序严格执行，**不要跳步**：

### Step 0: 幂等性检测

检查当前项目是否已执行过本命令：

1. 读取项目根目录的 `CLAUDE.md`，检查是否包含 `## 外部文档路径` 段落。
2. 检查 `.claude/memory/feedback_doc_output.md` 是否存在。

**如果以上任一条件成立**：
- 展示当前已有的配置信息（外部路径、记忆位置等）。
- 询问用户："检测到本项目已配置过 AI 工具。是否重新配置？(y/N)"
- 用户回答 N 或不回答 → 停止执行，输出当前配置摘要。
- 用户回答 Y → 继续执行后续步骤（已有文件仅更新，不删除）。

**如果都不成立**：继续执行。

---

### Step 1: 询问外部文档路径

向用户询问本项目的外部文档路径。AI 工具生成的文档、记忆同步等都将使用此路径。

**建议路径推导逻辑**：
- 如果 `$ARGUMENTS` 非空，建议路径：`~/yhz61010/Documents/$ARGUMENTS/`
- 如果 `$ARGUMENTS` 为空，尝试从 `git remote get-url origin` 提取仓库名作为项目名建议

**向用户展示**：
```
请输入本项目的外部文档路径。AI 工具生成的文档将统一保存在此路径下，
不同 AI 工具（Claude Code、Codex 等）可以共享此路径下的文档。

建议路径：<推导出的建议路径>

请输入路径（直接回车使用建议路径）：
```

**验证**：
- 必须是绝对路径（以 `/` 开头）
- `~` 需展开为实际 HOME 路径
- 去除尾部 `/`
- 如果路径不存在，询问："目录不存在，是否创建？(Y/n)"

将结果记为 `$EXT_DOC_PATH`，后续步骤使用。

---

### Step 2: 创建外部目录结构

使用 `mkdir -p` 创建以下标准目录结构：

```
$EXT_DOC_PATH/
├── Claude/                         # Claude Code 记忆同步目录
├── Codex/                          # Codex 记忆同步目录
│   └── current-codex-memories/     # Codex 当前记忆
├── Leo-Documents/                  # AI 生成的文档
│   └── superpowers/                # superpowers 插件产物
│       ├── plans/                  # 实现计划
│       └── specs/                  # 设计规范
```

**执行后汇报**：列出哪些目录是新创建的，哪些已经存在。

---

### Step 3: 询问 Git 用户信息

1. 先运行 `git config user.name` 和 `git config user.email` 读取当前项目配置。
2. **如果已设置**，展示当前值并询问："当前 Git 用户为 `<name> <email>`，是否沿用？(Y/n)"
   - 用户确认 → 记录并继续
   - 用户要修改 → 进入询问流程
3. **如果未设置**，使用默认值询问：
   ```
   请输入 Git 提交时使用的用户信息：
   - 用户名 (git user.name)（默认：Michael Leo）：
   - 邮箱 (git user.email)（默认：yhzemail61010@aliyun.com）：
   ```
   用户直接回车则使用默认值。
4. 通过 `git config user.name "<name>"` 和 `git config user.email "<email>"` 设置（**项目级**，不加 `--global`）。

将用户名记为 `$GIT_USER_NAME`，邮箱记为 `$GIT_USER_EMAIL`。

---

### Step 4: 询问项目类型与 GH_TOKEN 配置

首先询问项目类型：

```
本项目是 GitHub 项目还是内部项目？
1. GitHub 项目（代码托管在 GitHub）（默认）
2. 内部项目（代码托管在内部 GitLab/其他平台，或无远程仓库）

请选择 (1/2，默认 1)：
```

将用户选择记录为 `$PROJECT_TYPE`（`github` 或 `internal`）。

**如果用户选择"内部项目"** → 记录 `$GH_TOKEN_CONFIGURED = false`，跳到下一步。

**如果用户选择"GitHub 项目"**，继续询问 GH_TOKEN：

```
是否需要配置 GH_TOKEN 环境变量用于 GitHub API 操作？（默认：否）(y/N)
```

**如果用户选择"否"** → 记录 `$GH_TOKEN_CONFIGURED = false`，跳到下一步。

**如果用户选择"是"**：

1. 先展示 Token 生成指南：

```
如何在 GitHub 生成 Personal Access Token：

前往 https://github.com/settings/tokens

=== Fine-grained tokens（推荐，权限更精细） ===
1. 点击 "Generate new token" → "Fine-grained token"
2. 设置 Token name、Expiration（建议 90 天）
3. Resource owner：选择你的账户或组织
4. Repository access：选择 "Only select repositories" 并选中目标仓库
5. Permissions 设置：
   - Contents：Read and write（代码推送）
   - Pull requests：Read and write（创建/更新 PR）
   - Actions：Read and write（触发/查看 workflow）
   - Metadata：Read-only（自动授予）
6. 点击 "Generate token"

=== Tokens (classic)（传统方式，权限较粗） ===
1. 点击 "Generate new token" → "Generate new token (classic)"
2. 设置 Note、Expiration
3. 勾选以下 scope：
   - repo（全部子项：repo:status, repo_deployment, public_repo, repo:invite, security_events）
   - workflow（更新 GitHub Actions workflow）
   - read:org（读取组织成员信息，仅在组织仓库需要时勾选）
4. 点击 "Generate token"
```

2. 询问用户 Token：
```
请输入你的 GitHub Token（输入后会保存为敏感记忆，不会同步到外部路径）：
```

3. **安全处理**：
   - 将 Token 保存为项目记忆，**标记为敏感信息**，**绝不同步到外部路径**
   - 提醒用户使用方式：
     ```
     使用方法：
     - gh 命令：设置环境变量 GH_TOKEN=<your-token>，gh 会自动读取
     - git push：GH_TOKEN=<your-token> git push https://<username>@github.com/<owner>/<repo>.git <branch>
     ```

记录 `$GH_TOKEN_CONFIGURED = true`。

---

### Step 5: 询问记忆存储位置

```
项目记忆（记录协作约定、工作流偏好等）保存在哪里？

1. 外部路径（$EXT_DOC_PATH/Claude/）（默认）
   - 记忆独立于代码管理，不进入 git 仓库
   - 项目内 .claude/memory/MEMORY.md 仅保留指向外部路径的说明

2. 项目内（.claude/memory/）
   - 记忆跟随代码仓库，可被 git 管理和协作者共享
   - 同时会同步到外部路径 $EXT_DOC_PATH/Claude/

请选择 (1/2，默认 1)：
```

将用户选择记录为 `$MEMORY_LOCATION`（`external` 或 `project`），对应路径记为 `$MEMORY_PATH`：
- 选 1（默认） → `$MEMORY_PATH` = `$EXT_DOC_PATH/Claude/`
- 选 2 → `$MEMORY_PATH` = 项目根目录下 `.claude/memory/`

---

### Step 6: 写入交互规则记忆

在 `$MEMORY_PATH` 下创建 `feedback_ai_interaction.md`：

```markdown
---
name: AI 交互规则
description: 遇到不明确的地方必须先问用户，不要自行假设或幻想
type: feedback
---

若有不明白或不明确的地方，一定要先问我。不要自己幻想或无中生有。

**Why:** 避免 AI 工具在信息不充分时产生错误输出或幻觉内容。

**How to apply:** 在回答前检查问题是否有歧义或缺少必要信息。如有不确定，先向用户确认，不要擅自填补。
```

**写入前检查**：如果文件已存在，比对内容。内容相同则跳过，不同则询问是否覆盖。

---

### Step 7: 写入各项配置记忆

在 `$MEMORY_PATH` 下创建以下记忆文件。每个文件写入前检查是否已存在（已存在且内容相同则跳过）。

**7a. `feedback_memory_sync_policy.md`**：

```markdown
---
name: 记忆同步策略
description: 保存记忆时，无敏感信息自动同步到外部路径；有敏感信息不同步并醒目告知
type: feedback
---

保存新记忆时，执行敏感信息检查后决定是否同步到外部路径 `$EXT_DOC_PATH/Claude/`。

**敏感信息定义：** API Token、密码、密钥、个人凭据、内部 IP/域名、数据库连接串、
以及包含 token/password/secret/key/credential 等关键词或疑似 Base64 编码的长字符串。

**无敏感信息 →** 自动同步到 `$EXT_DOC_PATH/Claude/`，并同步更新 MEMORY.md 索引。

**有敏感信息 →** 不同步，并以如下格式醒目告知用户：

> **未同步到外部路径**
> 文件：`<文件名>`
> 原因：检测到敏感信息类型 — <具体类型，如 API Token、数据库连接串等>
> 记忆仅保留在 Claude Code 内部记忆中，未写入 `$EXT_DOC_PATH/Claude/`。

**Why:** 防止凭据、密钥等敏感内容泄露到外部共享路径。

**How to apply:** 每次保存记忆前，扫描内容是否含敏感关键词。无敏感 → 同步。有敏感 → 仅存本地并告知。
```

**7b. `feedback_claude_md_sync.md`**：

```markdown
---
name: CLAUDE.md 同步到外部路径
description: 每次更新项目中的 CLAUDE.md 时，同时复制一份到外部文档路径
type: feedback
---

每次更新项目中的 CLAUDE.md 时，同时复制到 `$EXT_DOC_PATH/CLAUDE.md`。

**Why:** 用户希望项目关键文件统一管理在外部文档目录下，便于不同 AI 工具共享。

**How to apply:** 修改 CLAUDE.md 后，执行复制到 `$EXT_DOC_PATH/CLAUDE.md`。
```

**7c. `feedback_doc_output.md`**：

```markdown
---
name: 文档输出位置
description: AI 生成的文档和 superpowers 插件产物保存到外部路径的 Leo-Documents/
type: feedback
---

AI 生成的文档默认保存到 `$EXT_DOC_PATH/Leo-Documents/`。

superpowers 插件及其 skills（brainstorming、writing-plans 等）生成的文件保存到：
`$EXT_DOC_PATH/Leo-Documents/superpowers/`
- specs → `$EXT_DOC_PATH/Leo-Documents/superpowers/specs/`
- plans → `$EXT_DOC_PATH/Leo-Documents/superpowers/plans/`

其他插件或 skills 生成的文档也应保存到 `$EXT_DOC_PATH/Leo-Documents/` 下对应子目录。

**Why:** 用户希望所有 AI 生成的文档统一管理在外部文档目录下，不放在项目仓库内。

**How to apply:** 生成文档时默认使用上述路径。用户显式指定其他路径时，以用户为准。
```

**7d. `feedback_memory_location.md`**：

```markdown
---
name: 记忆存放位置
description: 本项目的记忆文件存放位置配置
type: feedback
---

本项目的记忆存放在：`$MEMORY_PATH`

[如果选择了项目内：]
同时同步到 `$EXT_DOC_PATH/Claude/`（经过敏感信息检查后）。

[如果选择了外部路径：]
项目内 `.claude/memory/MEMORY.md` 仅保留指向外部路径的说明。

**Why:** 根据用户选择确定记忆文件的主存储位置。

**How to apply:** 所有 memory 文件的读写路径使用 `$MEMORY_PATH`。新会话开始时读取 `$EXT_DOC_PATH/Claude/MEMORY.md` 唤醒记忆。
```

**7e. `user_language.md`**：

```markdown
---
name: 中文交流偏好
description: 用户偏好使用中文进行对话交流
type: user
---

用户希望所有对话都使用中文进行。
```

**重要**：以上文件中的 `$EXT_DOC_PATH` 和 `$MEMORY_PATH` 替换为 Step 1 和 Step 5 获取的**实际绝对路径**。

---

### Step 8: 创建 MEMORY.md 索引

在 `$MEMORY_PATH` 下创建（或更新）`MEMORY.md` 索引文件。

**格式**（每条一行，遵循已有模式）：
```markdown
- [AI 交互规则](feedback_ai_interaction.md) — 遇到不确定的地方先问用户，不要幻想
- [记忆同步策略](feedback_memory_sync_policy.md) — 无敏感信息自动同步，有敏感信息不同步并告知
- [CLAUDE.md 同步](feedback_claude_md_sync.md) — 更新 CLAUDE.md 时同步到外部路径
- [文档输出位置](feedback_doc_output.md) — 生成文档和 superpowers 产物保存到外部路径
- [记忆存放位置](feedback_memory_location.md) — 本项目记忆的存放位置
- [中文交流偏好](user_language.md) — 用户偏好使用中文对话
```

如果 GH_TOKEN 已配置（Step 4），还需添加：
```markdown
- [GH_TOKEN 配置](feedback_gh_token.md) — 使用 GH_TOKEN 环境变量进行 GitHub 操作（敏感，未同步）
```

**同步逻辑**：
- 如果记忆在项目内 → 将非敏感的记忆文件和 MEMORY.md 同步到 `$EXT_DOC_PATH/Claude/`
- 如果记忆在外部 → 在项目内 `.claude/memory/MEMORY.md` 写入以下内容：
  ```markdown
  本项目记忆存储在外部路径，请读取以下文件获取完整记忆：
  $EXT_DOC_PATH/Claude/MEMORY.md
  ```

---

### Step 9: 更新/创建项目 CLAUDE.md

在项目根目录的 `CLAUDE.md` 中**追加**以下段落。如果 CLAUDE.md 不存在，创建新文件。如果已存在同名段落（`## 外部文档路径` 等），询问用户是否覆盖。

**追加内容**：

```markdown
## 外部文档路径

本项目的 AI 生成文档统一管理在：`$EXT_DOC_PATH/`

| 路径 | 用途 |
|------|------|
| `$EXT_DOC_PATH/Claude/` | Claude Code 记忆同步 |
| `$EXT_DOC_PATH/Codex/` | Codex 记忆同步 |
| `$EXT_DOC_PATH/Leo-Documents/` | AI 生成的文档 |
| `$EXT_DOC_PATH/Leo-Documents/superpowers/` | superpowers 插件产物（specs/、plans/） |
| `$EXT_DOC_PATH/CLAUDE.md` | 同步的 CLAUDE.md 副本 |

## AI 交互规则

- 若有不明白或不明确的地方，一定要先问我。不要自己幻想或无中生有。
- 用户偏好使用中文对话。

## 项目记忆

项目级记忆存储在 `$MEMORY_PATH` 目录中，包含跨会话的协作约定和工作流偏好。

新会话开始时，请先读取 `$EXT_DOC_PATH/Claude/MEMORY.md` 了解已有的记忆内容，便于重新唤醒记忆。
```

**写入后**，将 CLAUDE.md 同步复制到 `$EXT_DOC_PATH/CLAUDE.md`。

---

### Step 10: 配置新会话读取外部记忆（已在 Step 9 中完成）

此步骤的内容已包含在 Step 9 的"项目记忆"段落中。确认 CLAUDE.md 中已写入新会话读取指引即可。

---

### Step 11: 最终总结输出

执行完毕后，向用户展示完整的配置总结：

```
═══════════════════════════════════════════════════════
  AI 工具初始化配置完成
═══════════════════════════════════════════════════════

外部文档根路径：$EXT_DOC_PATH/
├── Claude/                         → Claude Code 记忆同步
├── Codex/                          → Codex 记忆同步
│   └── current-codex-memories/     → Codex 当前记忆
├── Leo-Documents/                  → AI 生成的文档
│   └── superpowers/                → superpowers 插件产物
│       ├── plans/                  → 实现计划
│       └── specs/                  → 设计规范
├── CLAUDE.md                       → 同步的 CLAUDE.md 副本
└── [其他项目特定目录]

记忆存储位置：$MEMORY_PATH
  [如果项目内] 同步目标：$EXT_DOC_PATH/Claude/
  [如果外部]   项目内索引：.claude/memory/MEMORY.md（指向外部路径）
新会话唤醒：读取 $EXT_DOC_PATH/Claude/MEMORY.md

项目配置文件：
├── CLAUDE.md                       → 已更新（含外部路径、交互规则、记忆配置）
└── $MEMORY_PATH/
    ├── MEMORY.md                   → 记忆索引
    ├── feedback_ai_interaction.md  → AI 交互规则
    ├── feedback_memory_sync_policy.md → 敏感信息检查与同步策略
    ├── feedback_claude_md_sync.md  → CLAUDE.md 同步规则
    ├── feedback_doc_output.md      → 文档输出位置
    ├── feedback_memory_location.md → 记忆存放位置
    └── user_language.md            → 中文交流偏好

Git 用户：$GIT_USER_NAME <$GIT_USER_EMAIL>
GH_TOKEN：[已配置（敏感，未同步到外部路径） / 未配置]

自动提示语：已启用
  "若有不明白或不明确的地方，一定要先问我。不要自己幻想或无中生有。"

═══════════════════════════════════════════════════════
```

**注意**：总结中的 `$EXT_DOC_PATH`、`$MEMORY_PATH`、`$GIT_USER_NAME`、`$GIT_USER_EMAIL` 等占位符必须替换为实际值。

## 边界与禁止事项

- 不执行 `git add` / `git commit` / `git push`，本命令不涉及任何 git 提交操作
- 不将 Token、密码等敏感信息写入可能被提交的文件
- 不覆盖已有记忆文件（仅新增或在用户确认后更新）
- 不修改全局 `~/.claude/CLAUDE.md`（只修改项目级 CLAUDE.md）
- 不安装或配置插件 / skills
- 外部路径使用绝对路径，去除尾部斜杠
- 所有占位符（`$EXT_DOC_PATH` 等）在实际写入文件时必须替换为真实路径
