#include "TrafficLightWidget.h"
#include <QPainter>
#include <QPainterPath>

TrafficLightWidget::TrafficLightWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePreset(Small);
    setAttribute(Qt::WA_TranslucentBackground);
}

void TrafficLightWidget::setSizePreset(SizePreset preset)
{
    m_sizePreset = preset;
    setFixedSize(sizeForPreset(preset));
    update();
}

QSize TrafficLightWidget::sizeForPreset(SizePreset preset) const
{
    switch (preset) {
    case Small:  return {80, 200};
    case Medium: return {100, 260};
    case Large:  return {130, 340};
    }
    return {80, 200};
}

void TrafficLightWidget::setAnimationMode(const QString &mode)
{
    m_animationMode = mode;
    if (m_state != LightState::Idle)
        startAnimation();
}

void TrafficLightWidget::setAnimationPeriodMs(int ms)
{
    m_animationPeriodMs = ms;
    if (m_state != LightState::Idle)
        startAnimation();
}

qreal TrafficLightWidget::activeAlpha() const
{
    return m_activeAlpha;
}

void TrafficLightWidget::setActiveAlpha(qreal alpha)
{
    m_activeAlpha = alpha;
    update();
}

void TrafficLightWidget::onStateChanged(LightState newState)
{
    m_state = newState;

    if (m_state == LightState::Idle) {
        stopAnimation();
        m_activeAlpha = 1.0;
    } else {
        startAnimation();
    }
    update();
}

void TrafficLightWidget::startAnimation()
{
    stopAnimation();

    m_animation = new QPropertyAnimation(this, "activeAlpha", this);
    m_animation->setDuration(m_animationPeriodMs);
    m_animation->setLoopCount(-1); // infinite

    if (m_animationMode == "breathing") {
        m_animation->setStartValue(0.3);
        m_animation->setKeyValueAt(0.5, 1.0);
        m_animation->setEndValue(0.3);
        m_animation->setEasingCurve(QEasingCurve::InOutSine);
    } else {
        // Classic blink: step function between 0 and 1
        m_animation->setStartValue(0.0);
        m_animation->setKeyValueAt(0.49, 0.0);
        m_animation->setKeyValueAt(0.50, 1.0);
        m_animation->setEndValue(1.0);
    }

    m_animation->start();
}

void TrafficLightWidget::stopAnimation()
{
    if (m_animation) {
        m_animation->stop();
        m_animation->deleteLater();
        m_animation = nullptr;
    }
}

QColor TrafficLightWidget::colorForLight(LightState light, bool active) const
{
    const QColor dimColor(60, 60, 60);
    if (!active)
        return dimColor;

    switch (light) {
    case LightState::Working:       return QColor(220, 40, 40);
    case LightState::WaitingConfirm: return QColor(240, 200, 20);
    case LightState::Idle:          return QColor(40, 200, 40);
    }
    return dimColor;
}

void TrafficLightWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();
    const int margin = w / 10;
    const int lightDiameter = (w - margin * 2) * 4 / 5;
    const int spacing = (h - margin * 2 - lightDiameter * 3) / 4;

    // Background housing
    QPainterPath housing;
    housing.addRoundedRect(QRectF(0, 0, w, h), w / 6.0, w / 6.0);
    painter.fillPath(housing, QColor(45, 45, 45));

    // Draw three lights: Red (top), Yellow (middle), Green (bottom)
    const LightState lights[] = {LightState::Working, LightState::WaitingConfirm, LightState::Idle};
    for (int i = 0; i < 3; ++i) {
        const int cx = w / 2;
        const int cy = margin + spacing + lightDiameter / 2 + i * (lightDiameter + spacing);

        const bool isActive = (lights[i] == m_state);
        QColor color = colorForLight(lights[i], isActive);

        if (isActive && m_state != LightState::Idle) {
            // Apply alpha for animation
            color.setAlphaF(m_activeAlpha);
        } else if (!isActive) {
            color.setAlphaF(0.3);
        }

        // Glow effect for active light
        if (isActive && m_activeAlpha > 0.5) {
            QRadialGradient glow(cx, cy, lightDiameter * 0.7);
            QColor glowColor = color;
            glowColor.setAlphaF(m_activeAlpha * 0.3);
            glow.setColorAt(0, glowColor);
            glow.setColorAt(1, Qt::transparent);
            painter.setBrush(glow);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(cx, cy), lightDiameter * 0.7, lightDiameter * 0.7);
        }

        // Light circle
        painter.setBrush(color);
        painter.setPen(QPen(QColor(30, 30, 30), 2));
        painter.drawEllipse(QPointF(cx, cy), lightDiameter / 2.0, lightDiameter / 2.0);
    }
}
