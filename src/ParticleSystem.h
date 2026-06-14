#pragma once

#include <QColor>
#include <QPointF>
#include <QVector>
#include <QPainterPath>

/// Type of parametric curve for particle animation.
enum class CurveType {
    Lemniscate,   ///< Bernoulli lemniscate (figure-8)
    Lissajous,    ///< Lissajous figure (aX:bY)
    Spiral,       ///< Archimedean spiral
};

/// Parameters for a parametric-curve particle animation.
struct ParticleParams {
    CurveType curveType = CurveType::Lissajous;

    // Particle system (common)
    int particleCount = 60;     ///< number of particles in trail
    qreal trailSpan = 0.34f;    ///< fraction of cycle spanned by the trail
    int durationMs = 6000;      ///< time for one full lap
    int pulseDurationMs = 5400; ///< time for one breathing cycle

    /// Amp controls how much of the circle radius the curve fills.
    /// finalAmp = ampBase + pulseScale * ampPulse
    qreal ampBase = 0.70f;
    qreal ampPulse = 0.25f;

    // Particle appearance (all relative to circle radius)
    qreal minParticleR = 0.04f;
    qreal maxParticleR = 0.16f;

    QColor color = QColor(255, 60, 60);

    // --- Lissajous-specific ---
    int lissajousAX = 3;
    int lissajousBY = 4;
    qreal lissajousPhase = 1.5708f;  // π/2
    qreal lissajousYScale = 0.92f;

    // --- Spiral-specific ---
    int spiralTurns = 4;
    /// Normalised base / amplitude / pulse  (the HTML values scaled so the
    /// point fits in [-1, +1] after dividing by the max possible radius).
    qreal spiralBase  = 0.27f;
    qreal spiralAmp   = 0.57f;
    qreal spiralPulse = 0.16f;
};

/// Per-particle render data, already mapped to widget coordinates.
struct ParticleData {
    QPointF pos;
    qreal opacity;
    qreal radius; // in widget pixels
};

/// Stateless particle engine: given time in ms returns particle positions.
///
/// The curve is evaluated in a normalized [-1, +1] space, then scaled to
/// fit inside a circle of the given radius centred at (cx, cy).
class ParticleSystem {
public:
    explicit ParticleSystem(const ParticleParams &params = ParticleParams());

    /// Compute particle positions for a given wall-clock time.
    QVector<ParticleData> compute(qreal timeMs,
                                  qreal cx, qreal cy, qreal r) const;

    /// Build the full curve path at the given time (widget coords).
    QPainterPath buildPath(qreal timeMs,
                           qreal cx, qreal cy, qreal r,
                           int steps = 480) const;

    const ParticleParams &params() const { return m_params; }
    void setParams(const ParticleParams &params) { m_params = params; }

private:
    ParticleParams m_params;

    /// Evaluate curve at progress [0, 1), returning normalized [-1, +1].
    void eval(qreal progress, qreal scale,
              qreal &outX, qreal &outY) const;

    static qreal wrap01(qreal v);
    qreal pulseScale(qreal timeMs) const;
};
