#pragma once

#include <QObject>
#include <QPointF>
#include <QVector>
#include <QTimer>
#include <QRectF>
#include <QColor>
#include <QPainter>

namespace DrawOnScreen {

struct LaserPoint {
    QPointF pos;
    qint64 timestamp = 0;
    qreal opacity = 1.0;
    qreal radius = 8.0;
};

class LaserPointerEngine : public QObject {
    Q_OBJECT
public:
    explicit LaserPointerEngine(QObject* parent = nullptr);

    void addPoint(const QPointF& pos, qreal pressure = 1.0);
    void clear();
    void render(QPainter& painter, const QColor& laserColor = QColor(255, 30, 60));
    QRectF currentBounds() const;
    bool isActive() const { return !m_points.isEmpty(); }

    void setFadeDuration(qint64 ms) { m_fadeDurationMs = ms; }
    void setRefreshRate(int hz);

signals:
    void frameReady(const QRectF& dirtyRect);

private slots:
    void tick();

private:
    QVector<LaserPoint> m_points;
    QTimer m_frameTimer;
    qint64 m_fadeDurationMs = 500;
    QRectF m_lastDirtyRect;
};

} // namespace DrawOnScreen
