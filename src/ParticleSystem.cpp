#include "ParticleSystem.h"
#include <QtMath>
#include <algorithm>

ParticleSystem::ParticleSystem(const ParticleParams &params)
    : m_params(params)
{
}

// ---------------------------------------------------------------------------
// Curve evaluation (normalized to [-1, +1])
// ---------------------------------------------------------------------------

void ParticleSystem::eval(qreal progress, qreal scale,
                          qreal &outX, qreal &outY) const
{
    const qreal t = progress * 2.0 * M_PI;
    const qreal amp = m_params.ampBase + scale * m_params.ampPulse;

    switch (m_params.curveType) {

    case CurveType::Lemniscate: {
        // Bernoulli lemniscate:  x = a·cos(t)/(1+sin²(t)), y = a·sin(t)cos(t)/(1+sin²(t))
        // Normalise: |cos(t)/denom| ≤ 1, |sin(t)cos(t)/denom| ≤ 0.5
        const qreal denom = 1.0 + std::sin(t) * std::sin(t);
        outX = (std::cos(t) / denom) * amp;
        outY = (std::sin(t) * std::cos(t) / denom) * amp;
        break;
    }

    case CurveType::Lissajous: {
        outX = std::sin(m_params.lissajousAX * t + m_params.lissajousPhase) * amp;
        outY = std::sin(m_params.lissajousBY * t) * amp * m_params.lissajousYScale;
        break;
    }

    case CurveType::Spiral: {
        // Archimedean spiral:  θ(t) = turns·t,  r(t) = base + (1-cos(t))·(amp + pulse·s)
        // Normalise so that max radius maps to ~1
        const qreal angle = t * m_params.spiralTurns;
        const qreal rawR = m_params.spiralBase
            + (1.0 - std::cos(t))
              * (m_params.spiralAmp + scale * m_params.spiralPulse);
        const qreal maxR = m_params.spiralBase
            + 2.0 * (m_params.spiralAmp + m_params.spiralPulse);
        const qreal normR = rawR / maxR;
        outX = std::cos(angle) * normR * amp;
        outY = std::sin(angle) * normR * amp;
        break;
    }
    }
}

// ---------------------------------------------------------------------------
// Time helpers
// ---------------------------------------------------------------------------

/*static*/ qreal ParticleSystem::wrap01(qreal v)
{
    v = std::fmod(v, qreal(1.0));
    return v < 0 ? v + 1.0 : v;
}

qreal ParticleSystem::pulseScale(qreal timeMs) const
{
    const qreal pulse = std::fmod(timeMs, qreal(m_params.pulseDurationMs))
                        / m_params.pulseDurationMs;
    const qreal angle = pulse * 2.0 * M_PI;
    // Same breathing curve as the HTML demos: sin maps to [0.52, 1.0]
    return 0.52 + ((std::sin(angle + 0.55) + 1.0) / 2.0) * 0.48;
}

// ---------------------------------------------------------------------------
// Per-frame computation
// ---------------------------------------------------------------------------

QVector<ParticleData> ParticleSystem::compute(qreal timeMs,
                                               qreal cx, qreal cy,
                                               qreal r) const
{
    if (m_params.particleCount < 1)
        return {};

    const qreal scale = pulseScale(timeMs);
    const qreal progress = wrap01(timeMs / m_params.durationMs);

    const int n = m_params.particleCount;
    QVector<ParticleData> data(n);

    for (int i = 0; i < n; ++i) {
        const qreal trail = qreal(i) / qreal(n - 1);
        const qreal p = wrap01(progress - trail * m_params.trailSpan);

        qreal nx, ny;
        eval(p, scale, nx, ny);

        const qreal fade = std::pow(1.0 - trail, 0.56f);

        data[i].pos     = QPointF(cx + nx * r, cy + ny * r);
        data[i].opacity = 0.04 + fade * 0.96;
        data[i].radius  = m_params.minParticleR
                          + fade * (m_params.maxParticleR - m_params.minParticleR);
        data[i].radius *= r;
    }

    return data;
}

// ---------------------------------------------------------------------------
// Full curve path
// ---------------------------------------------------------------------------

QPainterPath ParticleSystem::buildPath(qreal timeMs,
                                        qreal cx, qreal cy,
                                        qreal r, int steps) const
{
    const qreal scale = pulseScale(timeMs);
    QPainterPath path;

    for (int i = 0; i <= steps; ++i) {
        const qreal p = qreal(i) / steps;
        qreal nx, ny;
        eval(p, scale, nx, ny);

        const qreal px = cx + nx * r;
        const qreal py = cy + ny * r;

        if (i == 0)
            path.moveTo(px, py);
        else
            path.lineTo(px, py);
    }

    return path;
}
