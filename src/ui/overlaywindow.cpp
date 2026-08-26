#include "overlaywindow.h"
#include "canvasitem.h"
#include "../core/toolstatemanager.h"
#include "../core/document.h"
#include <LayerShellQt/window.h>
#include <QQmlContext>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QScreen>
#include <QFile>

namespace DrawOnScreen {

OverlayWindow::OverlayWindow(ToolStateManager* stateMgr, CanvasDocument* doc, QWindow* parent)
    : QQuickView(parent)
    , m_stateMgr(stateMgr)
    , m_document(doc)
{
    qmlRegisterType<CanvasItem>("DrawOnScreen", 1, 0, "CanvasItem");

    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);
    setColor(QColor(Qt::transparent));
    setResizeMode(QQuickView::SizeRootObjectToView);

    if (auto* scr = QGuiApplication::primaryScreen()) {
        setGeometry(scr->geometry());
    }

    m_layerWindow = LayerShellQt::Window::get(this);
    if (m_layerWindow) {
        m_layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
        m_layerWindow->setAnchors(
            LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop) |
            LayerShellQt::Window::AnchorBottom |
            LayerShellQt::Window::AnchorLeft |
            LayerShellQt::Window::AnchorRight
        );
        m_layerWindow->setExclusiveZone(-1);
        m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
        m_layerWindow->setScope(QStringLiteral("screen-annotation-overlay"));
    }

    rootContext()->setContextProperty(QStringLiteral("toolState"), m_stateMgr);
    rootContext()->setContextProperty(QStringLiteral("canvasDoc"), m_document);
    rootContext()->setContextProperty(QStringLiteral("overlayWindow"), this);

    connect(m_document, &CanvasDocument::undoRedoAvailabilityChanged, this, [this]() {
        m_stateMgr->setCanUndo(m_document->canUndo());
        m_stateMgr->setCanRedo(m_document->canRedo());
    });

    connect(m_stateMgr, &ToolStateManager::requestUndo, m_document, &CanvasDocument::undo);
    connect(m_stateMgr, &ToolStateManager::requestRedo, m_document, &CanvasDocument::redo);
    connect(m_stateMgr, &ToolStateManager::requestClear, m_document, &CanvasDocument::clear);
    connect(m_stateMgr, &ToolStateManager::requestExit, this, []() {
        QGuiApplication::quit();
    });

    connect(m_stateMgr, &ToolStateManager::interactionModeChanged, this, &OverlayWindow::applyMaskForMode);

    if (QFile::exists(QStringLiteral(":/qml/Overlay.qml"))) {
        setSource(QUrl(QStringLiteral("qrc:/qml/Overlay.qml")));
    } else {
        setSource(QUrl::fromLocalFile(QStringLiteral("src/qml/Overlay.qml")));
    }

    setTitle(QStringLiteral("DrawOnScreen"));
    applyMaskForMode();
}

OverlayWindow::~OverlayWindow() = default;

void OverlayWindow::updateScreenGeometry()
{
    if (auto* scr = screen()) {
        setGeometry(scr->geometry());
    } else if (auto* primary = QGuiApplication::primaryScreen()) {
        setGeometry(primary->geometry());
    }
    applyMaskForMode();
}

void OverlayWindow::resizeEvent(QResizeEvent* ev)
{
    QQuickView::resizeEvent(ev);
    applyMaskForMode();
}

void OverlayWindow::closeEvent(QCloseEvent* ev)
{
    ev->accept();
    QGuiApplication::quit();
}

void OverlayWindow::keyPressEvent(QKeyEvent* ev)
{
    const QKeySequence pressedSeq(ev->modifiers() | ev->key());

    if (pressedSeq == QKeySequence(m_stateMgr->shortcutExit()) ||
        ((ev->modifiers() & Qt::AltModifier) && ev->key() == Qt::Key_F4)) {
        QGuiApplication::quit();
        ev->accept();
        return;
    }

    if (pressedSeq == QKeySequence(m_stateMgr->shortcutToggleMode()) || ev->key() == Qt::Key_F9) {
        m_stateMgr->toggleInteractionMode();
        ev->accept();
        return;
    }

    if (pressedSeq == QKeySequence(m_stateMgr->shortcutUndo())) {
        m_document->undo();
        ev->accept();
        return;
    }

    if (pressedSeq == QKeySequence(m_stateMgr->shortcutRedo())) {
        m_document->redo();
        ev->accept();
        return;
    }

    if (pressedSeq == QKeySequence(m_stateMgr->shortcutClear())) {
        m_document->clear();
        ev->accept();
        return;
    }

    if (pressedSeq == QKeySequence(m_stateMgr->shortcutScreenshot())) {
        m_stateMgr->screenshotRequested();
        ev->accept();
        return;
    }

    if (ev->key() == Qt::Key_Escape) {
        m_stateMgr->setInteractionMode(InteractionMode::DesktopPassthrough);
        ev->accept();
        return;
    }

    if (ev->key() == Qt::Key_C && !(ev->modifiers() & Qt::ControlModifier)) {
        m_document->clear();
        ev->accept();
        return;
    }

    QQuickView::keyPressEvent(ev);
}

void OverlayWindow::setToolbarRect(const QRect& rect)
{
    if (m_toolbarRect != rect) {
        m_toolbarRect = rect;
        if (m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
            applyMaskForMode();
        }
    }
}

void OverlayWindow::applyMaskForMode()
{
    const bool isPassthrough = (m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough);

    if (isPassthrough) {
        // In desktop mode, mask ONLY the toolbar bounding box so toolbar is clickable
        // and everything else passes 100% through to desktop applications!
        setMask(QRegion(m_toolbarRect.isValid() ? m_toolbarRect : QRect(80, 80, 680, 54)));
        if (m_layerWindow) {
            m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
        }
    } else {
        // In drawing mode, full screen receives all input
        setMask(QRegion(0, 0, width(), height()));
        if (m_layerWindow) {
            m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
        }
    }
}

} // namespace DrawOnScreen
