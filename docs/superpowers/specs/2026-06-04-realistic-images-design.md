# 图片替换红绿灯 UI 设计文档

## 概述

将 TrafficLightWidget 的 QPainter 几何绘制替换为 PNG 图片渲染，让红绿灯看起来更真实。采用底图+亮灯图叠加方案，复用现有 QPropertyAnimation 动画架构。

## 图片资源

### 4 张 PNG

| 文件 | 内容 |
|------|------|
| `resources/images/light_off.png` | 全灭底图（外壳+三灯均灭） |
| `resources/images/light_red.png` | 红灯亮（完整红绿灯，红灯亮） |
| `resources/images/light_yellow.png` | 黄灯亮 |
| `resources/images/light_green.png` | 绿灯亮 |

要求：4 张图片尺寸一致，透明背景（PNG alpha），由用户提供。

### Qt Resource System

`resources/resources.qrc`：

```xml
<RCC>
  <qresource prefix="/images">
    <file>images/light_off.png</file>
    <file>images/light_red.png</file>
    <file>images/light_yellow.png</file>
    <file>images/light_green.png</file>
  </qresource>
</RCC>
```

CMake 中通过 `qt_add_resources` 编译进二进制。

## TrafficLightWidget 改造

### 新增成员

```cpp
QPixmap m_imgOff;
QPixmap m_imgRed;
QPixmap m_imgYellow;
QPixmap m_imgGreen;
QPixmap m_scaledOff;    // 缓存缩放后的图片
QPixmap m_scaledRed;
QPixmap m_scaledYellow;
QPixmap m_scaledGreen;
```

构造函数中从 qrc 路径加载原始图片。

### paintEvent 新逻辑

1. 绘制缩放后的全灭底图（`m_scaledOff`）
2. 根据当前状态选择亮灯图
3. `painter.setOpacity(m_activeAlpha)` 设置透明度
4. 叠加绘制亮灯图

### 动画机制不变

- QPropertyAnimation 驱动 m_activeAlpha — 完全复用
- 呼吸灯：0.3 → 1.0 → 0.3
- 经典闪烁：0.0 / 1.0 交替
- Idle（绿灯）：m_activeAlpha = 1.0 常亮不动画

### 移除的代码

- `colorForLight()` 方法
- paintEvent 中所有几何绘制（圆角矩形、圆形、辉光渐变）
- 灯直径、间距等计算逻辑

### 尺寸缩放

`setSizePreset()` 时预缩放 4 张 pixmap 到目标尺寸（Qt::KeepAspectRatio, Qt::SmoothTransformation），缓存到成员变量。paintEvent 直接绘制缓存 pixmap，不每帧缩放。

### 窗口尺寸

基于图片原始宽高比动态计算：
- Small：宽度 80px，高度按比例
- Medium：宽度 100px
- Large：宽度 130px

`sizeForPreset()` 不再硬编码高度。

## CMake 集成

`src/CMakeLists.txt` 中：

```cmake
qt_add_resources(trafficlight4ai RESOURCES ../resources/resources.qrc)
```

## 测试策略

无新增自动化测试。图片渲染手动验证。编译通过即验证 qrc 资源嵌入正确。
