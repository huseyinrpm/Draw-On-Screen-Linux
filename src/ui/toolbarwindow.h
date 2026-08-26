#pragma once

#include <QQuickView>
#include <QPoint>
#include <QRect>
#include <QMargins>
#include <QCloseEvent>
#include <QKeyEvent>

namespace LayerShellQt {
    class Window;
}

namespace DrawOnScreen {

class ToolStateManager;
class CanvasDocument;

class ToolbarWindow : public QQuickView {
    Q_OBJECT

public:
    explicit ToolbarWindow(ToolStateManager* stateMgr, CanvasDocument* doc, QWindow* parent = nullptr);
    ~ToolbarWindow() override;

    Q_INVOKABLE void startDrag();
    Q_INVOKABLE void updateDrag();
    Q_INVOKABLE void endDrag();
    Q_INVOKABLE void setToolbarPosition(int x, int y);
    Q_INVOKABLE void moveBy(int dx, int dy);
    Q_INVOKABLE void saveSettings();

    QRect currentToolbarRect() const;

protected:
    void closeEvent(QCloseEvent* ev) override;
    void keyPressEvent(QKeyEvent* ev) override;

public slots:
    void notifyGeometryChanged();

signals:
    void toolbarGeometryChanged(const QRect& geom);

private:
    void loadSavedPosition();

    ToolStateManager* m_stateMgr = nullptr;
    CanvasDocument* m_document = nullptr;
    LayerShellQt::Window* m_layerWindow = nullptr;
    QPoint m_currentPos = QPoint(80, 80);
    QPoint m_dragGlobalStart;
    QPoint m_windowStartPos;
    bool m_isDragging = false;
};

} // namespace DrawOnScreen
