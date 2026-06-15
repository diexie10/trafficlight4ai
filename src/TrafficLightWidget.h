#pragma once

#include <QWidget>
#include <QTimer>
#include <QVector>
#include "StateManager.h"
#include "ParticleSystem.h"

class TrafficLightWidget : public QWidget {
    Q_OBJECT

public:
    enum SizePreset { ExtraSmall, Small, Medium, Large, ExtraLarge };

    explicit TrafficLightWidget(QWidget *parent = nullptr);

    static SizePreset sizePresetFromString(const QString &size);
    void setSizePreset(SizePreset preset);
    void setAnimationMode(const QString &mode);
    void setAnimationPeriodMs(int ms);

    // Keep activeAlpha API for tray icon sync + test compatibility
    qreal activeAlpha() const;
    void setActiveAlpha(qreal alpha);
    void setRenderPaused(bool paused);

signals:
    void activeAlphaChanged(qreal alpha);

public slots:
    void onStateChanged(LightState newState);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onTick();

private:
    // Layout: three slots evenly spaced on a white rounded card
    struct Slot {
        qreal cx;       // centre X (pixels from paintEvent)
        qreal cy;       // centre Y (pixels from paintEvent)
        qreal r;        // circle radius (pixels from paintEvent)
        const ParticleSystem *sys;
    };
    void startAnimation();
    void stopAnimation();
    QSize sizeForPreset(SizePreset preset) const;

    // Background particle network
    struct BgParticle {
        qreal x, y;
        qreal vx, vy;
        qreal radius;
    };
    QVector<BgParticle> m_bgParticles;
    void initBgParticles();
    void drawBgParticles(QPainter &painter, qreal elapsedMs) const;

    // Card decorations
    void drawCardDecorations(QPainter &painter) const;

    // Rendering
    void drawWhiteCard(QPainter &painter) const;
    void drawSlot(QPainter &painter, const Slot &slot,
                  qreal elapsedMs, qreal activity) const;

    LightState m_state = LightState::Idle;
    LightState m_prevState = LightState::Idle;
    SizePreset m_sizePreset = Small;
    int m_animationPeriodMs = 1000;
    qreal m_activeAlpha = 1.0;

    // Render loop (dynamic rate)
    QTimer *m_renderTimer = nullptr;
    qint64 m_animationStartMs = 0;
    int m_idleFrameSkip = 0;

    // Fade transition between states
    qreal m_fadeProgress = 1.0;   // 0.0 = fading, 1.0 = done
    qint64 m_fadeStartMs = 0;
    static constexpr qreal FADE_DURATION_MS = 250.0;

    // Three particle systems
    ParticleSystem m_particleRed;    // left   — Lemniscate  (Working)
    ParticleSystem m_particleYellow; // centre — Lissajous   (WaitingConfirm)
    ParticleSystem m_particleGreen;  // right  — Spiral      (Idle)
};
