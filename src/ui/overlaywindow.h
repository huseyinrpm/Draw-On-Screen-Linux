#pragma once

#include <QQuickView>
#include <QRect>
#include <QCloseEvent>
#include <QKeyEvent>

namespace LayerShellQt {
    class Window;
}

namespace DrawOnScreen {

class ToolStateManager;
class CanvasDocument;
class LaserPointerEngine;

class OverlayWindow : public QQuickView {
    Q_OBJECT

public:
    explicit OverlayWindow(ToolStateManager* stateMgr, CanvasDocument* doc, LaserPointerEngine* laserEngine, QWindow* parent = nullptr);
    ~OverlayWindow() override;

    void updateScreenGeometry();

public slots:
    void setToolbarRect(const QRect& rect);
    void applyMaskForMode();

protected:
    void closeEvent(QCloseEvent* ev) override;
    void keyPressEvent(QKeyEvent* ev) override;
    void resizeEvent(QResizeEvent* ev) override;

private:
    ToolStateManager* m_stateMgr = nullptr;
    CanvasDocument* m_document = nullptr;
    LaserPointerEngine* m_laserEngine = nullptr;
    LayerShellQt::Window* m_layerWindow = nullptr;
    QRect m_toolbarRect = QRect(80, 80, 680, 54);
};

} // namespace DrawOnScreen
