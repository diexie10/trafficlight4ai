#include "TrafficLightWidget.h"
#include <QPainter>
#include <QDateTime>
#include <QtMath>
#include <QDebug>
#include <QRandomGenerator>

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
    m_renderTimer->start(16);

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
        p.color           = QColor(255, 184, 60);     // warm amber
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
    m_bgParticles.resize(35);
    auto *rg = QRandomGenerator::global();
    for (auto &p : m_bgParticles) {
        p.x      = rg->bounded(1.0);
        p.y      = rg->bounded(1.0);
        p.vx     = (rg->bounded(200) - 100) / 285714.0; // 0.35× of original
        p.vy     = (rg->bounded(200) - 100) / 285714.0;
        p.radius = rg->bounded(4, 11) / 10.0;
    }
}

void TrafficLightWidget::drawBgParticles(QPainter &painter,
                                          qreal elapsedMs) const
{
    const QRectF r = rect();
    if (r.isEmpty()) return;

    painter.save();
    painter.setClipRect(r);
    painter.setPen(Qt::NoPen);

    // Update positions deterministically from elapsed time
    const qreal dt = elapsedMs;
    struct { qreal x, y, rad; } pts[35];
    for (int i = 0; i < 35; ++i) {
        const auto &sp = m_bgParticles[i];
        qreal nx = std::fmod(sp.x + sp.vx * dt, 1.0);
        qreal ny = std::fmod(sp.y + sp.vy * dt, 1.0);
        if (nx < 0) nx += 1.0;
        if (ny < 0) ny += 1.0;
        pts[i].x   = r.x() + nx * r.width();
        pts[i].y   = r.y() + ny * r.height();
        pts[i].rad = sp.radius;
    }

    // --- Draw connecting lines between nearby particles ---
    const qreal maxDist = r.width() * 0.18;
    painter.setPen(QPen(QColor(0, 0, 0, 8), 0.3));
    for (int i = 0; i < 35; ++i) {
        for (int j = i + 1; j < 35; ++j) {
            const qreal dx = pts[i].x - pts[j].x;
            const qreal dy = pts[i].y - pts[j].y;
            const qreal dist = std::sqrt(dx * dx + dy * dy);
            if (dist < maxDist) {
                const qreal a = (1.0 - dist / maxDist) * 0.12;
                painter.setPen(QPen(QColor(0, 0, 0, static_cast<int>(a * 255)), 0.3));
                painter.drawLine(QPointF(pts[i].x, pts[i].y),
                                 QPointF(pts[j].x, pts[j].y));
            }
        }
    }

    // --- Draw particle dots ---
    for (int i = 0; i < 35; ++i) {
        painter.setBrush(QColor(0, 0, 0, 10));
        painter.drawEllipse(QPointF(pts[i].x, pts[i].y), pts[i].rad, pts[i].rad);
    }

    painter.restore();
}

// ============================================================================
// Card decorations (ambient glows, dividers, bottom line, corner dots)
// ============================================================================

void TrafficLightWidget::drawCardDecorations(QPainter &painter) const
{
    const QRectF r = rect();
    const qreal rad = qMin(r.width(), r.height()) * 0.10;
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
    const qreal glowR = r.width() * 0.22;
    struct { int ri, gi, bi; } glowColors[3] = {
        { 255, 80, 80 }, { 255, 184, 60 }, { 60, 200, 110 }
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
    // Keep 2:1 aspect ratio, scaled to 0.85× of original
    return {qRound(targetWidth * 0.85), qRound(targetWidth * 0.85 / 2)};
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

    // Breathing alpha for tray icon (0.3 … 1.0)
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

    update();
}

// ============================================================================
// State management
// ============================================================================

void TrafficLightWidget::onStateChanged(LightState newState)
{
    qDebug("[Widget] onStateChanged: %d -> %d",
           static_cast<int>(m_state), static_cast<int>(newState));
    m_state = newState;
    update();
}

// ============================================================================
// Animation control — always on
// ============================================================================

void TrafficLightWidget::startAnimation()
{
    m_animationStartMs = QDateTime::currentMSecsSinceEpoch();
    m_renderTimer->start(16);
}

void TrafficLightWidget::stopAnimation()
{
    // keep running — all three curves animate regardless of state
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
    m_activeAlpha = alpha;
    emit activeAlphaChanged(m_activeAlpha);
}

// ============================================================================
// Rendering
// ============================================================================

void TrafficLightWidget::drawWhiteCard(QPainter &painter) const
{
    const QRectF r = rect();
    const qreal rad = qMin(r.width(), r.height()) * 0.10;

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

// Desaturate a color to a muted version (15% of original saturation)
static QColor desaturated(const QColor &c, qreal factor = 0.15)
{
    if (c.saturation() == 0) return c;
    int h, s, v, a;
    c.getHsv(&h, &s, &v, &a);
    return QColor::fromHsv(h, qBound(0, static_cast<int>(s * factor), 255), v, a);
}

void TrafficLightWidget::drawSlot(QPainter &painter, const Slot &slot,
                                   qreal elapsedMs) const
{
    // cx, cy, r are already in absolute pixel coordinates from paintEvent
    const qreal cx = slot.cx;
    const qreal cy = slot.cy;
    const qreal r  = slot.r;

    const qreal inactivePathAlpha  = 0.40;
    const qreal inactivePartAlpha  = 0.04;

    const ParticleSystem &sys = *slot.sys;
    const QColor origColor = sys.params().color;
    // Inactive slots use a desaturated (muted) version of the original colour
    const QColor color = slot.active ? origColor : desaturated(origColor, 0.30);

    const QPainterPath path = sys.buildPath(elapsedMs, cx, cy, r);

    if (slot.active) {
        // === ACTIVE slot: glow + center nucleus + particles ===

        // --- 1a. Outer glow layers (bloom around the curve) ---
        QColor glowColor = origColor;
        for (int i = 3; i >= 1; --i) {
            glowColor.setAlphaF(0.05 / i);
            painter.setPen(QPen(glowColor, 2.0 * i + 1.0));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }

        // --- 1b. Main curve path ---
        QColor pathColor = origColor;
        pathColor.setAlphaF(0.30);
        painter.setPen(QPen(pathColor, 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);

        // --- 2a. Particles (full opacity, original color) ---
        const auto data = sys.compute(elapsedMs, cx, cy, r);
        painter.setPen(Qt::NoPen);
        for (const auto &p : data) {
            QColor pc = origColor;
            pc.setAlphaF(p.opacity); // full strength per particle
            painter.setBrush(pc);
            painter.drawEllipse(p.pos, p.radius, p.radius);
        }

        // --- 2b. Bright center nucleus ---
        QColor nc = origColor;
        nc.setAlphaF(0.7);
        painter.setBrush(nc);
        painter.drawEllipse(QPointF(cx, cy), r * 0.07, r * 0.07);
    } else {
        // === INACTIVE slot: faint gray path + faint gray particles ===

        // --- 1. Faint desaturated curve path ---
        QColor pathColor = color;
        pathColor.setAlphaF(inactivePathAlpha);
        painter.setPen(QPen(pathColor, 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);

        // --- 2. Faint gray particles ---
        const auto data = sys.compute(elapsedMs, cx, cy, r);
        painter.setPen(Qt::NoPen);
        for (const auto &p : data) {
            QColor pc = color;
            pc.setAlphaF(p.opacity * inactivePartAlpha);
            painter.setBrush(pc);
            painter.drawEllipse(p.pos, p.radius, p.radius);
        }

        // --- 3. Faint desaturated centre dot ---
        QColor nc = desaturated(origColor, 0.35);
        nc.setAlphaF(0.12);
        painter.setBrush(nc);
        painter.drawEllipse(QPointF(cx, cy), r * 0.04, r * 0.04);
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

    s.cx = sectionW * 0.5;
    s.sys = &m_particleRed;
    s.active = (m_state == LightState::Working);
    drawSlot(painter, s, elapsed);

    s.cx = sectionW * 1.5;
    s.sys = &m_particleYellow;
    s.active = (m_state == LightState::WaitingConfirm);
    drawSlot(painter, s, elapsed);

    s.cx = sectionW * 2.5;
    s.sys = &m_particleGreen;
    s.active = (m_state == LightState::Idle);
    drawSlot(painter, s, elapsed);
}
