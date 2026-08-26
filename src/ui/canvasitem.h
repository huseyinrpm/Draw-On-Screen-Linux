#pragma once

#include <QQuickPaintedItem>
#include <QPainterPath>
#include <QVector>
#include <QPointF>
#include <QRectF>
#include <QTimer>
#include "../core/types.h"
#include "../core/document.h"
#include "../core/laserengine.h"
#include "../core/toolstatemanager.h"

namespace DrawOnScreen {

class CanvasItem : public QQuickPaintedItem {
    Q_OBJECT

    Q_PROPERTY(DrawOnScreen::CanvasDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(DrawOnScreen::ToolStateManager* stateManager READ stateManager WRITE setStateManager NOTIFY stateManagerChanged)
    Q_PROPERTY(DrawOnScreen::LaserPointerEngine* laserEngine READ laserEngine WRITE setLaserEngine NOTIFY laserEngineChanged)
    Q_PROPERTY(QRect toolbarRect READ toolbarRect WRITE setToolbarRect NOTIFY toolbarRectChanged)

public:
    explicit CanvasItem(QQuickItem* parent = nullptr);
    ~CanvasItem() override;

    CanvasDocument* document() const { return m_document; }
    void setDocument(CanvasDocument* doc);

    ToolStateManager* stateManager() const { return m_stateMgr; }
    void setStateManager(ToolStateManager* mgr);

    LaserPointerEngine* laserEngine() const { return m_laserEngine; }
    void setLaserEngine(LaserPointerEngine* engine);

    QRect toolbarRect() const { return m_toolbarRect; }
    void setToolbarRect(const QRect& rect);

    void paint(QPainter* painter) override;

    Q_INVOKABLE void commitCurrentText();
    Q_INVOKABLE void cancelCurrentText();

signals:
    void documentChanged();
    void stateManagerChanged();
    void laserEngineChanged();
    void toolbarRectChanged();

protected:
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void keyPressEvent(QKeyEvent* ev) override;

private slots:
    void onGhostTick();
    void onCursorBlink();
    void onLaserDirtyRect(const QRectF& dirtyRect);

private:
    void handleStartPoint(const QPointF& pos, qreal pressure, qreal tiltX = 0, qreal tiltY = 0);
    void handleMovePoint(const QPointF& pos, qreal pressure, qreal tiltX = 0, qreal tiltY = 0);
    void handleReleasePoint(const QPointF& pos);
    void performVectorErase(const QPointF& pos, qreal radius);

    CanvasDocument* m_document = nullptr;
    ToolStateManager* m_stateMgr = nullptr;
    LaserPointerEngine* m_laserEngine = nullptr;

    QRect m_toolbarRect;
    bool m_isDrawing = false;
    QVector<StrokePoint> m_activePoints;
    QPainterPath m_activePreviewPath;
    qreal m_lastPressure = 1.0;
    QPointF m_dragStartPos;
    QPointF m_lastMousePos;

    // Photoshop-style In-Place Text Editing
    bool m_isEditingText = false;
    QPointF m_textEditPos;
    QString m_currentEditText;
    QTimer m_textCursorTimer;
    bool m_cursorVisible = true;

    // Hayalet / Sönen Kalem (Ghost Pen)
    QVector<Stroke> m_ghostStrokes;
    QTimer m_ghostTimer;
};

} // namespace DrawOnScreen
