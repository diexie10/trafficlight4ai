#include "TrafficLightWidget.h"
#include <QPainter>
#include <QDateTime>
#include <QtMath>
#include <QDebug>
#include <QRandomGenerator>
#include <QVarLengthArray>

// ============================================================================
// File-level constants
// ============================================================================

constexpr int kBgParticleCount = 35;
constexpr double kBgParticleSpeedDivisor = 800000.0;
constexpr int kBgParticleRadiusMin = 4;
constexpr int kBgParticleRadiusMax = 11;
constexpr double kBgParticleMaxAlpha = 30.0;
constexpr double kCardCornerRadiusFactor = 0.10;
constexpr double kGlowRadiusFactor = 0.22;
constexpr int kRenderIntervalMs = 16;

// ============================================================================
// Construction
// ============================================================================

TrafficLightWidget::TrafficLightWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setSizePreset(Small);

    // 60 fps render timer — always running from the start
    m_renderTimer = new QTimer(this);
    m_renderTimer->setTimerType(Qt::PreciseTimer);
    connect(m_renderTimer, &QTimer::timeout, this, &TrafficLightWidget::onTick);
    m_animationStartMs = QDateTime::currentMSecsSinceEpoch();
    m_renderTimer->start(kRenderIntervalMs);

    // ---- Left   – Red   – Bernoulli Lemniscate ---------------------------
    {
        ParticleParams p;
        p.curveType       = CurveType::Lemniscate;
        p.particleCount   = 70;
        p.trailSpan       = 0.40f;
        p.durationMs      = 5600;
        p.pulseDurationMs = 5000;
        p.ampBase         = 0.72f;
        p.ampPulse        = 0.26f;
        p.minParticleR    = 0.04f;
        p.maxParticleR    = 0.16f;
        p.color           = QColor(255, 80, 80);      // vibrant coral
        m_particleRed.setParams(p);
    }

    // ---- Centre – Yellow – Lissajous 3:4 ---------------------------------
    {
        ParticleParams p;
        p.curveType       = CurveType::Lissajous;
        p.lissajousAX     = 3;
        p.lissajousBY     = 4;
        p.lissajousPhase  = 1.5708f;    // π/2
        p.lissajousYScale = 0.92f;
        p.particleCount   = 68;
        p.trailSpan       = 0.34f;
        p.durationMs      = 6000;
        p.pulseDurationMs = 5400;
        p.ampBase         = 0.72f;
        p.ampPulse        = 0.28f;
        p.minParticleR    = 0.04f;
        p.maxParticleR    = 0.16f;
        p.color           = QColor(252, 211, 55);     // warm amber – balanced midpoint
        m_particleYellow.setParams(p);
    }

    // ---- Right – Green – Archimedean Spiral ------------------------------
    {
        ParticleParams p;
        p.curveType       = CurveType::Spiral;
        p.spiralTurns     = 4;
        p.spiralBase      = 0.27f;
        p.spiralAmp       = 0.57f;
        p.spiralPulse     = 0.16f;
        p.particleCount   = 86;
        p.trailSpan       = 0.28f;
        p.durationMs      = 7800;
        p.pulseDurationMs = 6800;
        p.ampBase         = 0.72f;
        p.ampPulse        = 0.28f;
        p.minParticleR    = 0.04f;
        p.maxParticleR    = 0.16f;
        p.color           = QColor(60, 200, 110);     // fresh green
        m_particleGreen.setParams(p);
    }

    // Background floating particle network
    initBgParticles();
}

// ============================================================================
// Background particle network
// ============================================================================

void TrafficLightWidget::initBgParticles()
{
    m_bgParticles.resize(kBgParticleCount);
    auto *rg = QRandomGenerator::global();
    for (auto &p : m_bgParticles) {
        p.x      = rg->bounded(1.0);
        p.y      = rg->bounded(1.0);
        p.vx     = (rg->bounded(200) - 100) / kBgParticleSpeedDivisor; // 0.35× of original
        p.vy     = (rg->bounded(200) - 100) / kBgParticleSpeedDivisor;
        p.radius = rg->bounded(kBgParticleRadiusMin, kBgParticleRadiusMax) / 10.0;
    }
}

void TrafficLightWidget::drawBgParticles(QPainter &painter,
                                          qreal elapsedMs) const
{
    const QRectF r = rect();
    if (r.isEmpty()) return;
    const int n = m_bgParticles.size();
    if (n < 1) return;

    painter.save();
    painter.setClipRect(r);
    painter.setPen(Qt::NoPen);

    // Update positions deterministically from elapsed time
    const qreal dt = elapsedMs;
    QVarLengthArray<QPointF, 35> pos(n);
    QVarLengthArray<qreal, 35> rad(n);
    for (int i = 0; i < n; ++i) {
        const auto &sp = m_bgParticles[i];
        qreal nx = std::fmod(sp.x + sp.vx * dt, 1.0);
        qreal ny = std::fmod(sp.y + sp.vy * dt, 1.0);
        if (nx < 0) nx += 1.0;
        if (ny < 0) ny += 1.0;
        pos[i] = QPointF(r.x() + nx * r.width(), r.y() + ny * r.height());
        rad[i] = sp.radius;
    }

    // --- Draw connecting lines between nearby particles ---
    const qreal maxDist = r.width() * 0.18;
    QPen linePen(QColor(0, 0, 0, 0), 0.3);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const qreal dx = pos[i].x() - pos[j].x();
            const qreal dy = pos[i].y() - pos[j].y();
            const qreal dist = std::sqrt(dx * dx + dy * dy);
            if (dist < maxDist) {
                const int a = qRound((1.0 - dist / maxDist) * kBgParticleMaxAlpha); // 0…30
                linePen.setColor(QColor(0, 0, 0, a));
                painter.setPen(linePen);
                painter.drawLine(pos[i], pos[j]);
            }
        }
    }

    // --- Draw particle dots ---
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 10));
    for (int i = 0; i < n; ++i)
        painter.drawEllipse(pos[i], rad[i], rad[i]);

    painter.restore();
}

// ============================================================================
// Card decorations (ambient glows, dividers, bottom line, corner dots)
// ============================================================================

void TrafficLightWidget::drawCardDecorations(QPainter &painter) const
{
    const QRectF r = rect();
    const qreal rad = qMin(r.width(), r.height()) * kCardCornerRadiusFactor;
    const qreal secW = r.width() / 3.0;
    const qreal cy   = r.height() / 2.0;

    // --- Concentric circles around each slot (clipped to card) ---
    {
        painter.save();
        painter.setClipRect(r);
        painter.setClipping(true);
        const qreal circleR = r.width() * 0.6; // diameter = 1.2× width
        painter.setPen(QPen(QColor(0, 0, 0, 10), 0.5));
        painter.setBrush(Qt::NoBrush);
        for (int i = 0; i < 3; ++i) {
            const qreal cx = r.x() + secW * (i + 0.5);
            painter.drawEllipse(QPointF(cx, r.height() / 2.0), circleR, circleR);
        }
        painter.restore();
    }

    // --- Ambient colour glows behind the three curve slots ---
    {
        painter.save();
        painter.setClipRect(r);
        painter.setClipping(true);
        const qreal glowR = r.width() * kGlowRadiusFactor;
        struct { int ri, gi, bi; } glowColors[3] = {
            { 255, 80, 80 }, { 252, 211, 55 }, { 60, 200, 110 }
        };
        for (int i = 0; i < 3; ++i) {
            const qreal cx = r.x() + secW * (i + 0.5);
            const auto &gc = glowColors[i];
            QRadialGradient grad(cx, cy, glowR);
            grad.setColorAt(0.0, QColor(gc.ri, gc.gi, gc.bi, 14));
            grad.setColorAt(0.5, QColor(gc.ri, gc.gi, gc.bi, 5));
            grad.setColorAt(1.0, QColor(gc.ri, gc.gi, gc.bi, 0));
            painter.setPen(Qt::NoPen);
            painter.setBrush(grad);
            painter.drawEllipse(QPointF(cx, cy), glowR, glowR);
        }
        painter.restore();
    }

    // --- Vertical divider lines between slots ---
    for (int i = 1; i < 3; ++i) {
        const qreal lx = r.x() + secW * i;
        painter.setPen(QPen(QColor(0, 0, 0, 10), 0.5));
        painter.drawLine(QPointF(lx, r.y() + rad * 0.6),
                         QPointF(lx, r.bottom() - rad * 0.6));
    }

    // --- Top accent hairline ---
    painter.setPen(QPen(QColor(0, 0, 0, 5), 0.5));
    painter.drawLine(QPointF(r.x() + rad, r.y() + 1),
                     QPointF(r.right() - rad, r.y() + 1));
}

TrafficLightWidget::SizePreset TrafficLightWidget::sizePresetFromString(const QString &size)
{
    if (size == "xsmall") return ExtraSmall;
    if (size == "small")  return Small;
    if (size == "medium") return Medium;
    if (size == "large")  return Large;
    if (size == "xlarge") return ExtraLarge;
    return Small;
}

void TrafficLightWidget::setSizePreset(SizePreset preset)
{
    m_sizePreset = preset;
    setFixedSize(sizeForPreset(preset));
    update();
}

QSize TrafficLightWidget::sizeForPreset(SizePreset preset) const
{
    int targetWidth = 160;
    switch (preset) {
    case ExtraSmall: targetWidth = 66;  break;
    case Small:      targetWidth = 82;  break;
    case Medium:     targetWidth = 102; break;
    case Large:      targetWidth = 134; break;
    case ExtraLarge: targetWidth = 178; break;
    }
    // Keep 2:1 aspect ratio, scaled to 1.02× of original
    return {qRound(targetWidth * 1.02), qRound(targetWidth * 1.02 / 2)};
}

void TrafficLightWidget::setAnimationMode(const QString &mode)
{
    Q_UNUSED(mode);
    // No longer used — always particle style.
}

void TrafficLightWidget::setAnimationPeriodMs(int ms)
{
    m_animationPeriodMs = qMax(ms, 200);
}

// ============================================================================
// Timer tick – drives frame update + breathing alpha
// ============================================================================

void TrafficLightWidget::onTick()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qreal elapsed = static_cast<qreal>(now - m_animationStartMs);
    const qreal period  = static_cast<qreal>(m_animationPeriodMs);

    // Advance fade progress
    const bool wasFading = (m_fadeProgress < 1.0);
    if (wasFading) {
        m_fadeProgress = qMin(static_cast<qreal>(now - m_fadeStartMs) / FADE_DURATION_MS, 1.0);
    }

    // Breathing alpha for tray icon (0.3 … 1.0) — always computed
    qreal t = std::fmod(elapsed, period) / period;
    if (t < 0.0) t += 1.0;
    qreal alpha;
    if (t < 0.25)
        alpha = 0.3 + (t / 0.25) * 0.7;
    else if (t < 0.75)
        alpha = 1.0;
    else
        alpha = 1.0 - ((t - 0.75) / 0.25) * 0.7;
    setActiveAlpha(alpha);

    // Throttle repaint: idle + no fade → skip every 3 frames (~20 fps)
    if (m_state == LightState::Idle && !wasFading) {
        ++m_idleFrameSkip;
        if (m_idleFrameSkip < 3)
            return;
        m_idleFrameSkip = 0;
    }

    update();
}

// ============================================================================
// State management
// ============================================================================

void TrafficLightWidget::onStateChanged(LightState newState)
{
    if (newState == m_state) return;
    qDebug("[Widget] onStateChanged: %d -> %d",
           static_cast<int>(m_state), static_cast<int>(newState));
    m_prevState = m_state;
    m_state = newState;
    m_fadeStartMs = QDateTime::currentMSecsSinceEpoch();
    m_fadeProgress = 0.0;
    update();
}

// ============================================================================
// Animation control — always on
// ============================================================================

void TrafficLightWidget::startAnimation()
{
    m_animationStartMs = QDateTime::currentMSecsSinceEpoch();
    m_renderTimer->start(kRenderIntervalMs);
}

void TrafficLightWidget::stopAnimation()
{
    m_renderTimer->stop();
}

void TrafficLightWidget::setRenderPaused(bool paused)
{
    if (paused)
        m_renderTimer->stop();
    else if (!m_renderTimer->isActive())
        m_renderTimer->start(kRenderIntervalMs);
}

// ============================================================================
// activeAlpha compatibility  (drives tray icon blinking)
// ============================================================================

qreal TrafficLightWidget::activeAlpha() const
{
    return m_activeAlpha;
}

void TrafficLightWidget::setActiveAlpha(qreal alpha)
{
    if (qFuzzyCompare(m_activeAlpha, alpha)) return;
    m_activeAlpha = alpha;
    emit activeAlphaChanged(m_activeAlpha);
}

// ============================================================================
// Rendering
// ============================================================================

void TrafficLightWidget::drawWhiteCard(QPainter &painter) const
{
    const QRectF r = rect();
    const qreal rad = qMin(r.width(), r.height()) * kCardCornerRadiusFactor;

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // --- Outer tray shadow ---
    QRectF tray = r.adjusted(-1.5, -1.5, 1.5, 1.5);
    painter.setBrush(QColor(0, 0, 0, 8));
    painter.drawRoundedRect(tray, rad + 1, rad + 1);

    // --- Warm cream card body ---
    painter.setBrush(QColor(246, 245, 243)); // #f6f5f3
    painter.setPen(QPen(QColor(0, 0, 0, 18), 0.5));
    painter.drawRoundedRect(r, rad, rad);
}

void TrafficLightWidget::drawSlot(QPainter &painter, const Slot &slot,
                                   qreal elapsedMs, qreal activity) const
{
    // cx, cy, r are already in absolute pixel coordinates from paintEvent
    const qreal cx = slot.cx;
    const qreal cy = slot.cy;
    const qreal r  = slot.r;

    const ParticleSystem &sys = *slot.sys;
    const QColor origColor = sys.params().color;

    // Interpolate color: desatured ↔ full
    // Interpolate path alpha: inactive (0.40) ↔ active (0.30)
    // Interpolate particle factor: inactive (0.04) ↔ active (1.0)
    // Interpolate center dot alpha: 0.12 ↔ 0.70
    // Interpolate path pen: 2.0 ↔ 1.5
    // Interpolate center dot radius: r*0.04 ↔ r*0.07
    const qreal a = qBound(0.0, activity, 1.0);
    const QColor color = QColor::fromHsv(
        origColor.hue(),
        qRound(origColor.saturation() * (0.30 + a * 0.70)),
        origColor.value(),
        origColor.alpha());
    const qreal pathAlpha   = 0.40 + a * (0.30 - 0.40);   // 0.40→0.30
    const qreal partFactor  = 0.04 + a * (1.0 - 0.04);    // 0.04→1.0
    const qreal dotAlpha    = 0.12 + a * (0.70 - 0.12);   // 0.12→0.70
    const qreal penWidth    = 2.0  + a * (1.5 - 2.0);     // 2.0→1.5
    const qreal dotR        = r * (0.04 + a * (0.07 - 0.04)); // 0.04r→0.07r

    const QPainterPath path = sys.buildPath(elapsedMs, cx, cy, r);

    // --- Glow layers: only when nearly fully active ---
    if (a > 0.85) {
        const qreal glowStrength = (a - 0.85) / 0.15; // 0→1 over 0.85→1.0
        QColor glowColor = origColor;
        for (int i = 3; i >= 1; --i) {
            glowColor.setAlphaF((0.05 / i) * glowStrength);
            painter.setPen(QPen(glowColor, 2.0 * i + 1.0));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }
    }

    // --- Main curve path ---
    {
        QColor pathColor = color;
        pathColor.setAlphaF(pathAlpha);
        painter.setPen(QPen(pathColor, penWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }

    // --- Particles ---
    {
        const auto data = sys.compute(elapsedMs, cx, cy, r);
        painter.setPen(Qt::NoPen);
        for (const auto &p : data) {
            QColor pc = color;
            pc.setAlphaF(p.opacity * partFactor);
            painter.setBrush(pc);
            painter.drawEllipse(p.pos, p.radius, p.radius);
        }
    }

    // --- Centre dot ---
    {
        QColor nc = color;
        nc.setAlphaF(dotAlpha);
        painter.setBrush(nc);
        painter.drawEllipse(QPointF(cx, cy), dotR, dotR);
    }
}

// ============================================================================
// paintEvent
// ============================================================================

void TrafficLightWidget::paintEvent(QPaintEvent *)
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qreal elapsed = static_cast<qreal>(now - m_animationStartMs);
    const qreal w = static_cast<qreal>(width());
    const qreal h = static_cast<qreal>(height());
    const qreal cy = h * 0.5;
    const qreal sectionW = w / 3.0;
    const qreal cr = sectionW * 0.35; // circle radius

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. White rounded card
    drawWhiteCard(painter);

    // 2. Background floating particle network
    drawBgParticles(painter, elapsed);

    // 3. Card decorations (ambient glows, dividers, bottom line, corner dots)
    drawCardDecorations(painter);

    // 4. Three curve slots: Red (left) | Yellow (centre) | Green (right)
    Slot s;
    s.cy = cy;
    s.r  = cr;

    // Helper lambda: does this ParticleSystem match a given state?
    auto sysForState = [&](const ParticleSystem *sys, LightState st) -> bool {
        return (sys == &m_particleRed && st == LightState::Working)
            || (sys == &m_particleYellow && st == LightState::WaitingConfirm)
            || (sys == &m_particleGreen && st == LightState::Idle);
    };

    s.cx = sectionW * 0.5;
    s.sys = &m_particleRed;
    {
        const bool isNow  = sysForState(s.sys, m_state);
        const bool wasThen = sysForState(s.sys, m_prevState);
        const qreal act = isNow ? m_fadeProgress : (wasThen ? 1.0 - m_fadeProgress : 0.0);
        drawSlot(painter, s, elapsed, act);
    }

    s.cx = sectionW * 1.5;
    s.sys = &m_particleYellow;
    {
        const bool isNow  = sysForState(s.sys, m_state);
        const bool wasThen = sysForState(s.sys, m_prevState);
        const qreal act = isNow ? m_fadeProgress : (wasThen ? 1.0 - m_fadeProgress : 0.0);
        drawSlot(painter, s, elapsed, act);
    }

    s.cx = sectionW * 2.5;
    s.sys = &m_particleGreen;
    {
        const bool isNow  = sysForState(s.sys, m_state);
        const bool wasThen = sysForState(s.sys, m_prevState);
        const qreal act = isNow ? m_fadeProgress : (wasThen ? 1.0 - m_fadeProgress : 0.0);
        drawSlot(painter, s, elapsed, act);
    }
}
