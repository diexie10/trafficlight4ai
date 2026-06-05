# 托盘图标在切换到绿灯时不变绿

> 日期：2026-06-05

## 背景 / 问题现象

为 TrayIcon 添加了与悬浮窗同步的闪烁动画后（通过 `activeAlphaChanged` signal 驱动 `setIcon`），发现红灯/黄灯闪烁正常，但切换到绿灯（Idle）时，托盘图标停留在红色或黄色，不会变成绿色。

通过 qDebug 日志确认 `setIcon(createIcon(绿色))` 确实被调用了：

```
[TrayIcon] onStateChanged: 0 -> 2
[TrayIcon] setIcon color=(40,200,40) alpha=1.0
[Widget] onStateChanged: 0 -> 2
[Widget] set alpha=1.0 directly (no signal)
```

代码逻辑正确，但视觉上托盘图标没有刷新。

## 最终可行方案

两处修改配合解决：

**1. 调整信号连接顺序**（`src/main.cpp`）

让 TrayIcon 先收到 `stateChanged`，确保 `m_state` 在 `activeAlphaChanged` 到达前已更新：

```cpp
// trayIcon first so m_state is updated
// before lightWidget's animation stop triggers activeAlphaChanged
QObject::connect(&stateManager, &StateManager::stateChanged,
                 trayIcon, &TrayIcon::onStateChanged);
QObject::connect(&stateManager, &StateManager::stateChanged,
                 lightWidget, &TrafficLightWidget::onStateChanged);
```

**2. 停止动画前断开信号**（`src/TrafficLightWidget.cpp`）

防止 `m_animation->stop()` 触发残余的 alpha 回调覆盖绿色图标：

```cpp
void TrafficLightWidget::stopAnimation()
{
    if (m_animation) {
        m_animation->disconnect(); // 防止 stale alpha 更新
        m_animation->stop();
        m_animation->deleteLater();
        m_animation = nullptr;
    }
}
```

**3. 延迟重设图标强制刷新**（`src/TrayIcon.cpp`）

Ubuntu SNI/AppIndicator 托盘在快速频繁 `setIcon`（动画期间）后紧接一次静态 `setIcon`，可能不刷新。通过 150ms 延迟重设解决：

```cpp
void TrayIcon::onStateChanged(LightState newState)
{
    // ... 设置 m_state、m_currentColor、setIcon ...

    if (newState == LightState::Idle) {
        QTimer::singleShot(150, this, [this]() {
            setIcon(createIcon(m_currentColor));
        });
    }
}
```

## 踩过的坑

1. **仅在 `onActiveAlphaChanged` 中对 Idle 状态 return** —— 无效，因为信号连接顺序导致 `m_state` 在 alpha 回调时还未更新为 Idle，旧颜色被以 alpha=1.0 重设覆盖了绿色图标
2. **仅调整信号连接顺序** —— 解决了信号时序问题，但 Ubuntu SNI 托盘仍然不刷新图标缓存
3. **`QMenu::setParent(this)` 修复内存泄漏** —— `QSystemTrayIcon` 不是 `QWidget`，`QMenu::setParent` 需要 `QWidget*` 类型，编译失败

## 参考

- `src/TrayIcon.cpp:39-66` — onStateChanged 和 onActiveAlphaChanged 实现
- `src/TrafficLightWidget.cpp:113-120` — stopAnimation 断开信号
- `src/main.cpp:57-60` — 信号连接顺序
