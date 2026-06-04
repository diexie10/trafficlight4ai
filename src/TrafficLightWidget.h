#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPropertyAnimation>
#include "StateManager.h"

class TrafficLightWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal activeAlpha READ activeAlpha WRITE setActiveAlpha)

public:
    enum SizePreset { Small, Medium, Large };

    explicit TrafficLightWidget(QWidget *parent = nullptr);

    void setSizePreset(SizePreset preset);
    void setAnimationMode(const QString &mode);
    void setAnimationPeriodMs(int ms);

    qreal activeAlpha() const;
    void setActiveAlpha(qreal alpha);

public slots:
    void onStateChanged(LightState newState);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void startAnimation();
    void stopAnimation();
    void rescalePixmaps();
    QSize sizeForPreset(SizePreset preset) const;

    LightState m_state = LightState::Idle;
    SizePreset m_sizePreset = Small;
    QString m_animationMode = "breathing";
    int m_animationPeriodMs = 1000;
    qreal m_activeAlpha = 1.0;
    QPropertyAnimation *m_animation = nullptr;

    // Original images
    QPixmap m_imgOff;
    QPixmap m_imgRed;
    QPixmap m_imgYellow;
    QPixmap m_imgGreen;

    // Cached scaled images
    QPixmap m_scaledOff;
    QPixmap m_scaledRed;
    QPixmap m_scaledYellow;
    QPixmap m_scaledGreen;
};
