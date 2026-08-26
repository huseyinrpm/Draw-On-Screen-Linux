#pragma once

#include <QColor>
#include <QDateTime>
#include <QFont>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QUuid>
#include <QVector>

namespace DrawOnScreen {

enum class ToolType {
    Pen = 0,
    Highlighter,
    Laser,
    GhostPen,
    Eraser,
    Line,
    Arrow,
    Rectangle,
    Ellipse,
    Text
};

enum class InteractionMode {
    Drawing = 0,
    DesktopPassthrough
};

enum class BackgroundMode {
    Transparent = 0,
    Whiteboard,
    Blackboard
};

struct StrokePoint {
    QPointF pos;
    qreal pressure = 1.0;
    qreal tiltX = 0.0;
    qreal tiltY = 0.0;
    qint64 timestampMs = 0;

    StrokePoint() = default;
    StrokePoint(const QPointF &p, qreal pr = 1.0, qreal tx = 0.0, qreal ty = 0.0, qint64 ts = 0)
        : pos(p), pressure(pr), tiltX(tx), tiltY(ty)
        , timestampMs(ts > 0 ? ts : QDateTime::currentMSecsSinceEpoch()) {}
};

struct Stroke {
    QUuid id;
    ToolType tool = ToolType::Pen;
    QColor color = Qt::red;
    qreal baseWidth = 4.0;
    QVector<StrokePoint> points;
    QPainterPath cachedPath;
    QRectF cachedBoundingRect;
    bool isFilled = false;
    QString text;
    QFont font;
    qint64 createdAtMs = 0;
    qreal opacity = 1.0;
    bool isDirty = true;

    Stroke() : id(QUuid::createUuid()), createdAtMs(QDateTime::currentMSecsSinceEpoch()) {}
};

} // namespace DrawOnScreen
