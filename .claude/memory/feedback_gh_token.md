---
name: GH_TOKEN 配置
description: 使用 GH_TOKEN 环境变量进行 GitHub 操作（敏感，未同步到 docs/Claude/）
type: feedback
---

GitHub Token 已配置。使用方式：
- gh 命令：设置环境变量 `GH_TOKEN=<your-token>`，gh 会自动读取
- git push：`GH_TOKEN=<your-token> git push https://<username>@github.com/<owner>/<repo>.git <branch>`

Token 值存储在 Claude Code 内部，不写入任何可被 git 追踪的文件。

**Why:** 此环境使用共享电脑，不适合 `gh auth login` 持久登录，改用一次性环境变量。

**How to apply:** 需要 GitHub API 操作时，提醒用户设置 GH_TOKEN 环境变量。
