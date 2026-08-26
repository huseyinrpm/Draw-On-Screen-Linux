#pragma once

#include "types.h"
#include <QPainterPath>
#include <QVector>
#include <QPointF>

namespace DrawOnScreen {

class Smoother {
public:
    static QPainterPath buildSmoothPath(const QVector<StrokePoint>& points);
    static QPainterPath buildArrowPath(const QPointF& start, const QPointF& end, qreal headSize = 20.0);
    static QPainterPath buildLinePath(const QPointF& start, const QPointF& end);
    static QPainterPath buildRectPath(const QPointF& start, const QPointF& end);
    static QPainterPath buildEllipsePath(const QPointF& start, const QPointF& end);

    static qreal distanceToSegment(const QPointF& p, const QPointF& a, const QPointF& b);
    static bool strokeIntersects(const Stroke& stroke, const QPointF& eraserPos, qreal radius);
};

} // namespace DrawOnScreen
