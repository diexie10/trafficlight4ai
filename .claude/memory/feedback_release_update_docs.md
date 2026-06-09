---
name: 发布 Release 时同步更新文档链接
description: 每次发布新 Release 后，更新 README.md、README_zh.md 中的版本号和下载链接；BUILD 文档无需更新
type: feedback
---

发布新 Release 时，必须同步更新以下文件中的版本号和下载链接：
- README.md
- README_zh.md

**不需要更新的文件：**
- docs/BUILD.md — 已改为 `<version>` 占位符格式，无需每次更新
- docs/BUILD_zh.md — 同上

**Why:** README 中列出了具体的发布文件名和下载链接（指向 GitHub Releases），版本号硬编码在文件名和 URL 中。BUILD 文档已改为通用格式，不再硬编码版本号。

**How to apply:** 每次发布新版本后，在 README.md 和 README_zh.md 中搜索旧版本号，替换为新版本号，确保所有下载链接指向最新 release。
