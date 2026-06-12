---
description: 发布 GitHub Release：更新版本号、触发构建、更新 Release 说明
argument-hint: <version> (如 0.5.0)
---

# /github-release

发布一个新的 GitHub Release：更新 README 版本号 → 提交推送 → 触发构建 → 轮询等待 → 更新 Release 说明 → 同步 tag。

用户传入的参数：`$ARGUMENTS`

## 前置条件

- 当前分支为 `main`，工作区干净
- `GH_TOKEN` 环境变量已设置（用于 push 和 API 调用）

## 执行步骤

### Step 1: 解析参数与收集变更

1. 从 `$ARGUMENTS` 提取版本号（不带 `v` 前缀）。如果参数为空，停止并输出：
   > 用法：`/github-release <version>`，例如 `/github-release 0.5.0`
2. 检查工作区是否干净（`git status`），不干净则停止。
3. 运行 `git describe --tags --abbrev=0` 获取上一个 tag。
4. 运行 `git log <上一个tag>..HEAD --oneline` 收集变更列表。
5. 根据 commit 列表，**起草 Release Notes**（纯文本，按功能分组）。
6. **向用户汇报**变更摘要和拟写的 Release Notes，等待用户确认后再继续。

### Step 2: 更新 README 版本号

1. 从 README.md 的下载链接中提取**当前版本号**（如 `0.4.0`）。
2. 用 Edit 工具的 `replace_all` 将 README.md 和 README_zh.md 中所有旧版本号替换为新版本号。

### Step 3: 提交并推送

```bash
git add README.md README_zh.md
git commit -m "docs: bump version to <version> in README download links"
GH_TOKEN=<token> git push https://<user>@github.com/<owner>/<repo>.git main
```

- owner/repo 从 `git remote get-url origin` 提取。
- user 从 remote URL 或 `git config user.name` 提取。

### Step 4: 触发 release-packages workflow 并轮询等待

1. 通过 GitHub API 触发 workflow：
   ```
   POST /repos/{owner}/{repo}/actions/workflows/release-packages.yml/dispatches
   { "ref": "main", "inputs": { "version": "<version>", "notes": "<notes>" } }
   ```

2. **关键约束**：`notes` 中**绝对不能包含反引号**（`` ` ``）。workflow 的 `${{ inputs.notes }}` 会被直接内联到 bash 脚本中，反引号会被 bash 解释为命令替换，导致构建失败。用普通文本替代所有代码标记。

3. **不要提前创建 tag 或 Release**。workflow 会自行创建 tag `v<version>` 和 Release。如果 tag 已存在，workflow 会失败。

4. 触发后，**每 10 秒轮询一次** workflow run 状态：
   ```
   GET /repos/{owner}/{repo}/actions/runs?per_page=1
   ```
   - 检查 `.workflow_runs[0].status` 和 `.workflow_runs[0].conclusion`
   - `status == "completed"` 且 `conclusion == "success"` → 进入 Step 5
   - `status == "completed"` 且 `conclusion == "failure"` → 报告失败，输出 `html_url` 供用户检查，停止执行
   - 其他 → 继续等待

### Step 5: 更新 Release 说明

构建成功后：

1. 通过 API 获取 Release 信息：
   ```
   GET /repos/{owner}/{repo}/releases/tags/v<version>
   ```
   提取 `id` 和 `assets` 列表。

2. 组装完整的 Release body（这里**可以使用反引号**，因为是通过 JSON API 直接写入，不经过 bash 解释）：
   - What's New 部分：使用 Markdown 格式（反引号、加粗等）
   - Downloads 部分：生成平台/文件名/链接的 Markdown 表格，包含所有 assets

3. 通过 API 更新 Release：
   ```
   PATCH /repos/{owner}/{repo}/releases/{id}
   { "body": "<formatted markdown>" }
   ```
   使用 heredoc (`-d @- <<'ENDJSON'`) 传递 JSON body，避免 shell 转义问题。

### Step 6: 同步本地 tag

```bash
git fetch origin --tags
```

确认 `v<version>` tag 已同步到本地。

## 输出结果

- 输出 Release 页面 URL
- 一句话确认完成

## 边界与禁止事项

- ❌ 不要在触发 workflow 前手动创建 tag 或 Release
- ❌ 不要在 workflow dispatch 的 notes 参数中使用反引号
- ❌ 不要在用户确认前执行推送或触发 workflow
- ✅ Release body 的 PATCH 请求可以使用反引号（JSON API，不经过 bash）
- ✅ 使用 heredoc 传递 JSON 避免转义问题
