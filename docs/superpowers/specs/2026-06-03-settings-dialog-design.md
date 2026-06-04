# 设置对话框设计文档

## 概述

为 trafficlight4cc 添加 SettingsDialog（QDialog），让用户通过 GUI 修改所有配置项，实时预览效果，取消可撤销。

## 方案

独立 QDialog，使用 Qt Widgets 原生控件，与现有代码风格一致。

## SettingsDialog 布局

使用 QFormLayout，4 行配置项：

| 配置项 | 控件 | 行为 |
|--------|------|------|
| 窗口大小 | QComboBox（小/中/大） | 切换后实时改变悬浮窗尺寸 |
| 动画模式 | QComboBox（呼吸灯/经典闪烁） | 切换后实时改变闪烁效果 |
| 动画周期 | QSlider + QLabel 显示毫秒值 | 范围 200~5000ms，步进 100ms，拖动实时预览 |
| Socket 路径 | QLineEdit | 失去焦点或按回车时应用，热重启 IPC Server |

### 底部按钮

- **确定** — 关闭对话框（配置已通过实时预览保存）
- **取消** — 恢复到打开对话框时的快照，关闭

### 窗口属性

- 固定大小，不可缩放
- 标题："设置 - Traffic Light for Claude Code"
- 单例：只允许同时打开一个设置对话框

## 实时预览与撤销

### 实时预览

SettingsDialog 接收 ConfigManager、TrafficLightWidget、IpcServer 指针。控件值变化时：

1. 窗口大小 — QComboBox::currentIndexChanged → ConfigManager::setWindowSize() + TrafficLightWidget::setSizePreset()
2. 动画模式 — QComboBox::currentIndexChanged → ConfigManager::setAnimationMode() + TrafficLightWidget::setAnimationMode()
3. 动画周期 — QSlider::valueChanged → ConfigManager::setAnimationPeriodMs() + TrafficLightWidget::setAnimationPeriodMs()
4. Socket 路径 — QLineEdit::editingFinished → ConfigManager::setSocketPath() + IpcServer::restart(newPath)

### 取消撤销

- 对话框打开时，快照当前 4 项配置值到局部变量
- 点击"取消"时，用快照值逐一恢复 ConfigManager 和 UI
- Socket 路径若被修改过，取消时用旧路径热重启 IPC Server

## 现有类修改

### IpcServer 新增

`restart(const QString &newPath)` 方法：
- 关闭当前 QLocalServer
- 删除旧 socket 文件
- 更新 m_socketPath
- 在新路径重新 listen

### ConfigManager 新增

`setSocketPath(const QString &path)` — 写入 socket.path 配置并保存

## 打开入口

两个入口打开同一个 SettingsDialog 实例：

1. 托盘右键菜单 — 在"显示/隐藏"和"退出"之间插入"设置"菜单项
2. 悬浮窗右键菜单 — FloatingWindow 添加 contextMenuEvent，右键弹出菜单包含"设置"和"退出"

### 单例控制

在 main.cpp 中创建一个 SettingsDialog 实例（隐藏状态），托盘和悬浮窗都指向同一个实例。打开时调用 show() + raise()。

## 测试策略

不对 SettingsDialog 做自动化 UI 测试。新增可测试点：

- test_ipc_server：IpcServer::restart() 重启后旧路径不可用、新路径可用
- test_config_manager：ConfigManager::setSocketPath() 读写正确
