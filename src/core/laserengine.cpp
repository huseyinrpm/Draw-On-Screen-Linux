#include "laserengine.h"
#include <QDateTime>
#include <QRadialGradient>
#include <algorithm>

namespace DrawOnScreen {

LaserPointerEngine::LaserPointerEngine(QObject* parent)
    : QObject(parent)
{
    connect(&m_frameTimer, &QTimer::timeout, this, &LaserPointerEngine::tick);
    m_frameTimer.setTimerType(Qt::PreciseTimer);
    m_frameTimer.start(1000 / 144); // Default 144Hz
}

void LaserPointerEngine::setRefreshRate(int hz)
{
    const int effectiveHz = qBound(30, hz, 240);
    m_frameTimer.setInterval(1000 / effectiveHz);
}

void LaserPointerEngine::addPoint(const QPointF& pos, qreal pressure)
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qreal r = 6.0 + 6.0 * qBound(0.2, pressure, 1.5);
    m_points.push_back({pos, now, 1.0, r});

    if (!m_frameTimer.isActive()) {
        m_frameTimer.start();
    }
}

void LaserPointerEngine::clear()
{
    const QRectF old = currentBounds();
    m_points.clear();
    if (!old.isNull()) {
        emit frameReady(old.adjusted(-10, -10, 10, 10));
    }
}

QRectF LaserPointerEngine::currentBounds() const
{
    if (m_points.isEmpty()) return QRectF();

    qreal minX = m_points[0].pos.x();
    qreal maxX = minX;
    qreal minY = m_points[0].pos.y();
    qreal maxY = minY;
    qreal maxR = m_points[0].radius;

    for (const auto& pt : m_points) {
        minX = qMin(minX, pt.pos.x());
        maxX = qMax(maxX, pt.pos.x());
        minY = qMin(minY, pt.pos.y());
        maxY = qMax(maxY, pt.pos.y());
        maxR = qMax(maxR, pt.radius);
    }

    return QRectF(QPointF(minX - maxR - 4, minY - maxR - 4),
                  QPointF(maxX + maxR + 4, maxY + maxR + 4));
}

void LaserPointerEngine::tick()
{
    if (m_points.isEmpty()) {
        if (!m_lastDirtyRect.isNull()) {
            QRectF toClear = m_lastDirtyRect;
            m_lastDirtyRect = QRectF();
            emit frameReady(toClear);
        }
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    QRectF newBounds = currentBounds();

    for (auto& pt : m_points) {
        const qint64 age = now - pt.timestamp;
        pt.opacity = qMax(0.0, 1.0 - static_cast<double>(age) / m_fadeDurationMs);
    }

    m_points.erase(
        std::remove_if(m_points.begin(), m_points.end(), [](const LaserPoint& p) {
            return p.opacity <= 0.005;
        }),
        m_points.end()
    );

    QRectF combined = newBounds.united(m_lastDirtyRect).adjusted(-10, -10, 10, 10);
    m_lastDirtyRect = currentBounds();

    emit frameReady(combined);
}

void LaserPointerEngine::render(QPainter& painter, const QColor& laserColor)
{
    if (m_points.isEmpty()) return;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw fading trail segments
    for (int i = 0; i < m_points.size() - 1; ++i) {
        const auto& p1 = m_points[i];
        const auto& p2 = m_points[i + 1];
        const qreal avgOpacity = (p1.opacity + p2.opacity) * 0.5;

        // Outer glow
        QColor outer = laserColor;
        outer.setAlphaF(0.35 * avgOpacity);
        QPen outerPen(outer, (p1.radius + p2.radius) * 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(outerPen);
        painter.drawLine(p1.pos, p2.pos);

        // Core line
        QColor core = QColor(255, 255, 255);
        core.setAlphaF(0.9 * avgOpacity);
        QPen corePen(core, (p1.radius + p2.radius) * 0.35, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(corePen);
        painter.drawLine(p1.pos, p2.pos);
    }

    // Draw pointer tip (last point) with radial glow
    const auto& tip = m_points.last();
    QRadialGradient grad(tip.pos, tip.radius * 2.2);
    QColor cCenter(255, 255, 255, static_cast<int>(255 * tip.opacity));
    QColor cMid = laserColor;
    cMid.setAlphaF(0.8 * tip.opacity);
    QColor cEdge = laserColor;
    cEdge.setAlphaF(0.0);

    grad.setColorAt(0.0, cCenter);
    grad.setColorAt(0.35, cMid);
    grad.setColorAt(1.0, cEdge);

    painter.setPen(Qt::NoPen);
    painter.setBrush(grad);
    painter.drawEllipse(tip.pos, tip.radius * 2.2, tip.radius * 2.2);

    painter.restore();
}

} // namespace DrawOnScreen
