#include "smoother.h"
#include <cmath>
#include <QtMath>

namespace DrawOnScreen {

QPainterPath Smoother::buildSmoothPath(const QVector<StrokePoint>& points)
{
    QPainterPath path;
    const int n = points.size();
    if (n == 0) return path;

    if (n == 1) {
        path.moveTo(points[0].pos);
        path.lineTo(points[0].pos + QPointF(0.1, 0.1));
        return path;
    }

    if (n == 2) {
        path.moveTo(points[0].pos);
        path.lineTo(points[1].pos);
        return path;
    }

    path.moveTo(points[0].pos);

    for (int i = 0; i < n - 1; ++i) {
        const QPointF p0 = points[qMax(0, i - 1)].pos;
        const QPointF p1 = points[i].pos;
        const QPointF p2 = points[i + 1].pos;
        const QPointF p3 = points[qMin(n - 1, i + 2)].pos;

        // Catmull-Rom spline to cubic Bézier control points
        const QPointF c1 = p1 + (p2 - p0) / 6.0;
        const QPointF c2 = p2 - (p3 - p1) / 6.0;
        path.cubicTo(c1, c2, p2);
    }

    return path;
}

QPainterPath Smoother::buildLinePath(const QPointF& start, const QPointF& end)
{
    QPainterPath path;
    path.moveTo(start);
    path.lineTo(end);
    return path;
}

QPainterPath Smoother::buildArrowPath(const QPointF& start, const QPointF& end, qreal headSize)
{
    QPainterPath path;
    path.moveTo(start);
    path.lineTo(end);

    const qreal dx = end.x() - start.x();
    const qreal dy = end.y() - start.y();
    const qreal length = std::hypot(dx, dy);

    if (length > 0.001) {
        const qreal angle = std::atan2(dy, dx);
        const qreal arrowAngle = M_PI / 6.0; // 30 degrees
        const qreal effectiveHeadSize = qMin(headSize, length * 0.4);

        const QPointF p1(end.x() - effectiveHeadSize * std::cos(angle - arrowAngle),
                         end.y() - effectiveHeadSize * std::sin(angle - arrowAngle));
        const QPointF p2(end.x() - effectiveHeadSize * std::cos(angle + arrowAngle),
                         end.y() - effectiveHeadSize * std::sin(angle + arrowAngle));

        path.moveTo(p1);
        path.lineTo(end);
        path.lineTo(p2);
    }

    return path;
}

QPainterPath Smoother::buildRectPath(const QPointF& start, const QPointF& end)
{
    QPainterPath path;
    const QRectF rect(QPointF(qMin(start.x(), end.x()), qMin(start.y(), end.y())),
                      QPointF(qMax(start.x(), end.x()), qMax(start.y(), end.y())));
    path.addRoundedRect(rect, 4.0, 4.0);
    return path;
}

QPainterPath Smoother::buildEllipsePath(const QPointF& start, const QPointF& end)
{
    QPainterPath path;
    const QRectF rect(QPointF(qMin(start.x(), end.x()), qMin(start.y(), end.y())),
                      QPointF(qMax(start.x(), end.x()), qMax(start.y(), end.y())));
    path.addEllipse(rect);
    return path;
}

qreal Smoother::distanceToSegment(const QPointF& p, const QPointF& a, const QPointF& b)
{
    const qreal l2 = (b.x() - a.x()) * (b.x() - a.x()) + (b.y() - a.y()) * (b.y() - a.y());
    if (l2 == 0.0) return std::hypot(p.x() - a.x(), p.y() - a.y());

    // Consider the line extending the segment, parameterized as a + t (b - a).
    // We find projection of point p onto the line.
    // It falls where t = [(p-a) . (b-a)] / |b-a|^2
    const qreal t = qMax(0.0, qMin(1.0, ((p.x() - a.x()) * (b.x() - a.x()) + (p.y() - a.y()) * (b.y() - a.y())) / l2));
    const QPointF projection = a + t * (b - a);
    return std::hypot(p.x() - projection.x(), p.y() - projection.y());
}

bool Smoother::strokeIntersects(const Stroke& stroke, const QPointF& eraserPos, qreal radius)
{
    // Bounding box fast reject
    const qreal totalThreshold = radius + (stroke.baseWidth * 0.5) + 4.0;
    if (!stroke.cachedBoundingRect.isNull()) {
        const QRectF expanded = stroke.cachedBoundingRect.adjusted(-totalThreshold, -totalThreshold, totalThreshold, totalThreshold);
        if (!expanded.contains(eraserPos)) {
            return false;
        }
    }

    if (stroke.points.size() < 2) {
        if (stroke.points.size() == 1) {
            return std::hypot(stroke.points[0].pos.x() - eraserPos.x(), stroke.points[0].pos.y() - eraserPos.y()) <= totalThreshold;
        }
        return false;
    }

    // Check segment distances
    for (int i = 0; i < stroke.points.size() - 1; ++i) {
        if (distanceToSegment(eraserPos, stroke.points[i].pos, stroke.points[i + 1].pos) <= totalThreshold) {
            return true;
        }
    }

    return false;
}

} // namespace DrawOnScreen
