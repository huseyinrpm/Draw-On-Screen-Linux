#include "toolbarwindow.h"
#include "../core/toolstatemanager.h"
#include "../core/document.h"
#include <LayerShellQt/window.h>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QFile>

namespace DrawOnScreen {

ToolbarWindow::ToolbarWindow(ToolStateManager* stateMgr, CanvasDocument* doc, QWindow* parent)
    : QQuickView(parent)
    , m_stateMgr(stateMgr)
    , m_document(doc)
{
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);
    setColor(QColor(Qt::transparent));
    setResizeMode(QQuickView::SizeViewToRootObject);

    loadSavedPosition();

    m_layerWindow = LayerShellQt::Window::get(this);
    if (m_layerWindow) {
        // Overlay layer sits above LayerTop canvas
        m_layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
        m_layerWindow->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop) | LayerShellQt::Window::AnchorLeft);
        m_layerWindow->setMargins(QMargins(m_currentPos.x(), m_currentPos.y(), 0, 0));
        m_layerWindow->setExclusiveZone(0);
        m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
        m_layerWindow->setScope(QStringLiteral("screen-annotation-toolbar"));
    }

    // Context properties
    rootContext()->setContextProperty(QStringLiteral("toolState"), m_stateMgr);
    rootContext()->setContextProperty(QStringLiteral("toolbarWindow"), this);

    // Sync undo/redo state
    connect(m_document, &CanvasDocument::undoRedoAvailabilityChanged, this, [this]() {
        m_stateMgr->setCanUndo(m_document->canUndo());
        m_stateMgr->setCanRedo(m_document->canRedo());
    });

    // Wire up tool state signals to document actions
    connect(m_stateMgr, &ToolStateManager::requestUndo, m_document, &CanvasDocument::undo);
    connect(m_stateMgr, &ToolStateManager::requestRedo, m_document, &CanvasDocument::redo);
    connect(m_stateMgr, &ToolStateManager::requestClear, m_document, &CanvasDocument::clear);
    connect(m_stateMgr, &ToolStateManager::requestExit, this, []() {
        QGuiApplication::quit();
    });

    connect(this, &QQuickView::statusChanged, this, [this](QQuickView::Status status) {
        if (status == QQuickView::Ready && rootObject()) {
            connect(rootObject(), SIGNAL(widthChanged()), this, SLOT(notifyGeometryChanged()));
            connect(rootObject(), SIGNAL(heightChanged()), this, SLOT(notifyGeometryChanged()));
            notifyGeometryChanged();
        }
    });

    // Load QML from resources or fallback to file system
    if (QFile::exists(QStringLiteral(":/qml/Toolbar.qml"))) {
        setSource(QUrl(QStringLiteral("qrc:/qml/Toolbar.qml")));
    } else {
        setSource(QUrl::fromLocalFile(QStringLiteral("src/qml/Toolbar.qml")));
    }

    setTitle(QStringLiteral("DrawOnScreen Toolbar"));
}

ToolbarWindow::~ToolbarWindow()
{
    saveSettings();
}

void ToolbarWindow::closeEvent(QCloseEvent* ev)
{
    ev->accept();
    QGuiApplication::quit();
}

void ToolbarWindow::keyPressEvent(QKeyEvent* ev)
{
    if ((ev->modifiers() & Qt::AltModifier) && ev->key() == Qt::Key_F4) {
        QGuiApplication::quit();
        ev->accept();
        return;
    }

    if (ev->key() == Qt::Key_F9) {
        m_stateMgr->toggleInteractionMode();
        ev->accept();
        return;
    }

    QQuickView::keyPressEvent(ev);
}

QRect ToolbarWindow::currentToolbarRect() const
{
    int w = 680;
    int h = 54;
    if (rootObject()) {
        if (rootObject()->width() > 0) w = static_cast<int>(rootObject()->width());
        if (rootObject()->height() > 0) h = static_cast<int>(rootObject()->height());
    }
    return QRect(m_currentPos.x(), m_currentPos.y(), w, h);
}

void ToolbarWindow::notifyGeometryChanged()
{
    QRect rect = currentToolbarRect();
    if (width() != rect.width() || height() != rect.height()) {
        resize(rect.width(), rect.height());
    }
    if (m_layerWindow) {
        m_layerWindow->setMargins(QMargins(m_currentPos.x(), m_currentPos.y(), 0, 0));
    }
    emit toolbarGeometryChanged(rect);
}

void ToolbarWindow::loadSavedPosition()
{
    QSettings settings(QStringLiteral("DrawOnScreen"), QStringLiteral("Toolbar"));
    const int x = settings.value(QStringLiteral("posX"), 80).toInt();
    const int y = settings.value(QStringLiteral("posY"), 80).toInt();
    m_currentPos = QPoint(x, y);
    setPosition(m_currentPos);
}

void ToolbarWindow::saveSettings()
{
    QSettings settings(QStringLiteral("DrawOnScreen"), QStringLiteral("Toolbar"));
    settings.setValue(QStringLiteral("posX"), m_currentPos.x());
    settings.setValue(QStringLiteral("posY"), m_currentPos.y());
}

void ToolbarWindow::startDrag()
{
    m_isDragging = true;
}

void ToolbarWindow::updateDrag()
{
    // updateDrag handled directly by instantaneous moveBy calls
}

void ToolbarWindow::moveBy(int dx, int dy)
{
    if (dx == 0 && dy == 0) return;

    int newX = m_currentPos.x() + dx;
    int newY = m_currentPos.y() + dy;

    if (auto* scr = QGuiApplication::primaryScreen()) {
        const QRect geom = scr->geometry();
        int w = width();
        int h = height();
        newX = qBound(0, newX, qMax(0, geom.width() - w));
        newY = qBound(0, newY, qMax(0, geom.height() - h));
    }

    if (m_currentPos.x() == newX && m_currentPos.y() == newY) return;

    m_currentPos = QPoint(newX, newY);
    if (m_layerWindow) {
        m_layerWindow->setMargins(QMargins(m_currentPos.x(), m_currentPos.y(), 0, 0));
    }
}

void ToolbarWindow::endDrag()
{
    m_isDragging = false;
    saveSettings();
    emit toolbarGeometryChanged(currentToolbarRect());
}

void ToolbarWindow::setToolbarPosition(int x, int y)
{
    if (auto* scr = QGuiApplication::primaryScreen()) {
        const QRect geom = scr->geometry();
        int w = width();
        int h = height();
        m_currentPos.setX(qBound(0, x, qMax(0, geom.width() - w)));
        m_currentPos.setY(qBound(0, y, qMax(0, geom.height() - h)));
    } else {
        m_currentPos = QPoint(x, y);
    }

    if (m_layerWindow) {
        m_layerWindow->setMargins(QMargins(m_currentPos.x(), m_currentPos.y(), 0, 0));
    }
    notifyGeometryChanged();
    saveSettings();
}

} // namespace DrawOnScreen
