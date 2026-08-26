#pragma once

#include <QRasterWindow>
#include <QPainterPath>
#include <QVector>
#include <QPointF>
#include <QRect>
#include <QCloseEvent>
#include <QTimer>
#include "../core/types.h"
#include "../core/document.h"
#include "../core/toolstatemanager.h"

namespace LayerShellQt {
    class Window;
}

namespace DrawOnScreen {

class CanvasWindow : public QRasterWindow {
    Q_OBJECT
public:
    explicit CanvasWindow(CanvasDocument* doc, ToolStateManager* stateMgr);
    ~CanvasWindow() override;

    void updateScreenGeometry();

protected:
    void paintEvent(QPaintEvent* ev) override;
    void tabletEvent(QTabletEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void keyPressEvent(QKeyEvent* ev) override;
    void resizeEvent(QResizeEvent* ev) override;
    void closeEvent(QCloseEvent* ev) override;

public slots:
    void onInteractionModeChanged(int mode);
    void onDocumentChanged();
    void setToolbarRect(const QRect& rect);
    void onGhostTick();
    void onCursorBlink();

private:
    void applyMaskForMode();
    void handleStartPoint(const QPointF& pos, qreal pressure, qreal tiltX = 0, qreal tiltY = 0);
    void handleMovePoint(const QPointF& pos, qreal pressure, qreal tiltX = 0, qreal tiltY = 0);
    void handleReleasePoint(const QPointF& pos);
    void performVectorErase(const QPointF& pos, qreal radius);

    void commitCurrentText();
    void cancelCurrentText();

    CanvasDocument* m_document = nullptr;
    ToolStateManager* m_stateMgr = nullptr;
    LayerShellQt::Window* m_layerWindow = nullptr;

    QRect m_toolbarRect;
    bool m_isDrawing = false;
    QVector<StrokePoint> m_activePoints;
    QPainterPath m_activePreviewPath;
    QRectF m_lastActiveDirtyRect;
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
