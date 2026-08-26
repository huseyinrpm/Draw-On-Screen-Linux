#include "canvaswindow.h"
#include "../core/smoother.h"
#include <LayerShellQt/window.h>
#include <QPainter>
#include <QTabletEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QDateTime>
#include <cmath>

namespace DrawOnScreen {

CanvasWindow::CanvasWindow(CanvasDocument* doc, ToolStateManager* stateMgr)
    : QRasterWindow()
    , m_document(doc)
    , m_stateMgr(stateMgr)
{
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);

    QSurfaceFormat fmt = format();
    fmt.setAlphaBufferSize(8);
    setFormat(fmt);

    if (auto* scr = QGuiApplication::primaryScreen()) {
        setGeometry(scr->geometry());
    }

    setTitle(QStringLiteral("DrawOnScreen Canvas"));

    m_layerWindow = LayerShellQt::Window::get(this);
    if (m_layerWindow) {
        // Use LayerTop so the floating toolbar on LayerOverlay is always physically above the canvas
        m_layerWindow->setLayer(LayerShellQt::Window::LayerTop);
        m_layerWindow->setAnchors(
            LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop) |
            LayerShellQt::Window::AnchorBottom |
            LayerShellQt::Window::AnchorLeft |
            LayerShellQt::Window::AnchorRight
        );
        m_layerWindow->setExclusiveZone(-1);
        m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
        m_layerWindow->setScope(QStringLiteral("screen-annotation-canvas"));
    }

    connect(m_stateMgr, &ToolStateManager::interactionModeChanged, this, &CanvasWindow::onInteractionModeChanged);
    connect(m_stateMgr, &ToolStateManager::backgroundModeChanged, this, [this](int) { update(); });
    connect(m_stateMgr, &ToolStateManager::areDrawingsVisibleChanged, this, [this](bool) { update(); });
    connect(m_document, &CanvasDocument::documentChanged, this, &CanvasWindow::onDocumentChanged);

    connect(&m_textCursorTimer, &QTimer::timeout, this, &CanvasWindow::onCursorBlink);
    connect(&m_ghostTimer, &QTimer::timeout, this, &CanvasWindow::onGhostTick);
    m_ghostTimer.setInterval(1000 / 60); // 60 FPS smooth fade

    applyMaskForMode();
}

CanvasWindow::~CanvasWindow() = default;

void CanvasWindow::updateScreenGeometry()
{
    if (auto* scr = screen()) {
        setGeometry(scr->geometry());
    } else if (auto* primary = QGuiApplication::primaryScreen()) {
        setGeometry(primary->geometry());
    }
    applyMaskForMode();
}

void CanvasWindow::resizeEvent(QResizeEvent* ev)
{
    QRasterWindow::resizeEvent(ev);
    applyMaskForMode();
}

void CanvasWindow::closeEvent(QCloseEvent* ev)
{
    ev->accept();
    QGuiApplication::quit();
}

void CanvasWindow::setToolbarRect(const QRect& rect)
{
    m_toolbarRect = rect;
    applyMaskForMode();
    update();
}

void CanvasWindow::onInteractionModeChanged(int mode)
{
    Q_UNUSED(mode);
    if (m_isEditingText && m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
        commitCurrentText();
    }
    applyMaskForMode();
    update();
}

void CanvasWindow::applyMaskForMode()
{
    const bool isPassthrough = (m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough);

    if (isPassthrough) {
        setCursor(Qt::ArrowCursor);
        // An off-screen 1x1 rect creates a non-null QRegion that produces an empty visible
        // Wayland wl_region. This guarantees 100% click-through to all desktop applications.
        setMask(QRegion(QRect(-100, -100, 1, 1)));
        if (m_layerWindow) {
            m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
        }
    } else {
        setCursor(Qt::CrossCursor);
        // Drawing mode: full screen EXCEPT the toolbar area!
        QRegion drawRegion(0, 0, width(), height());
        if (!m_toolbarRect.isNull() && m_toolbarRect.isValid()) {
            drawRegion = drawRegion.subtracted(QRegion(m_toolbarRect.adjusted(-4, -4, 4, 4)));
        }
        setMask(drawRegion);
        if (m_layerWindow) {
            m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
        }
    }
}

void CanvasWindow::onDocumentChanged()
{
    update();
}

void CanvasWindow::onCursorBlink()
{
    m_cursorVisible = !m_cursorVisible;
    if (m_isEditingText) {
        update();
    }
}

void CanvasWindow::onGhostTick()
{
    if (m_ghostStrokes.isEmpty()) {
        m_ghostTimer.stop();
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 lifetimeMs = m_stateMgr->ghostDurationMs();

    for (auto& s : m_ghostStrokes) {
        const qint64 age = now - s.createdAtMs;
        s.opacity = qMax(0.0, 1.0 - static_cast<double>(age) / lifetimeMs);
    }

    m_ghostStrokes.erase(
        std::remove_if(m_ghostStrokes.begin(), m_ghostStrokes.end(), [](const Stroke& s) {
            return s.opacity <= 0.02;
        }),
        m_ghostStrokes.end()
    );

    update();
}

void CanvasWindow::commitCurrentText()
{
    if (!m_isEditingText) return;

    if (!m_currentEditText.trimmed().isEmpty()) {
        Stroke s;
        s.tool = ToolType::Text;
        s.color = m_stateMgr->currentColor();
        s.text = m_currentEditText;
        QFont font;
        font.setPointSize(m_stateMgr->fontSize());
        font.setBold(true);
        s.font = font;
        s.points.push_back(StrokePoint(m_textEditPos));
        QFontMetrics fm(font);
        s.cachedBoundingRect = QRectF(m_textEditPos.x(), m_textEditPos.y() - fm.ascent(),
                                      fm.horizontalAdvance(m_currentEditText), fm.height());
        m_document->addStroke(s);
    }

    m_isEditingText = false;
    m_currentEditText.clear();
    m_textCursorTimer.stop();
    update();
}

void CanvasWindow::cancelCurrentText()
{
    m_isEditingText = false;
    m_currentEditText.clear();
    m_textCursorTimer.stop();
    update();
}

void CanvasWindow::paintEvent(QPaintEvent* ev)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRect clip = ev->rect();

    // 1. Background mode (Transparent / Whiteboard / Blackboard)
    if (m_stateMgr->areDrawingsVisible()) {
        const auto bgMode = m_stateMgr->backgroundMode();
        if (bgMode == BackgroundMode::Whiteboard) {
            p.fillRect(clip, QColor(248, 250, 252));
        } else if (bgMode == BackgroundMode::Blackboard) {
            p.fillRect(clip, QColor(15, 23, 42));
        }
    }

    // Clip out the toolbar rectangle so no drawing ever shows behind or over the menu
    if (!m_toolbarRect.isNull() && m_toolbarRect.isValid()) {
        p.setClipRegion(QRegion(clip).subtracted(QRegion(m_toolbarRect)));
    }

    // 2. Render committed strokes if visible
    if (m_stateMgr->areDrawingsVisible()) {
        m_document->render(p, clip);
    }

    // 3. Render Ghost Pen Fading Strokes
    for (const auto& s : m_ghostStrokes) {
        p.save();
        QColor ghostCol = s.color;
        ghostCol.setAlphaF(ghostCol.alphaF() * s.opacity);
        QPen pen(ghostCol, s.baseWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(s.cachedPath);
        p.restore();
    }

    // 4. Render active in-progress stroke / shape preview
    if (m_isDrawing && m_stateMgr->currentTool() != ToolType::Eraser) {
        const auto tool = m_stateMgr->currentTool();
        const QColor col = m_stateMgr->currentColor();
        const qreal baseW = m_stateMgr->currentWidth();

        if (tool == ToolType::Highlighter) {
            p.save();
            QColor hl = col;
            hl.setAlphaF(0.40);
            QPen pen(hl, baseW, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawPath(m_activePreviewPath);
            p.restore();
        } else if (tool == ToolType::GhostPen) {
            QPen pen(col, baseW * qMax(0.2, m_lastPressure), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawPath(m_activePreviewPath);
        } else {
            QPen pen(col, baseW * (tool == ToolType::Pen ? qMax(0.2, m_lastPressure) : 1.0),
                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            p.setPen(pen);

            if (m_stateMgr->isFilled() && (tool == ToolType::Rectangle || tool == ToolType::Ellipse)) {
                QColor fill = col;
                fill.setAlphaF(0.25);
                p.setBrush(fill);
            } else {
                p.setBrush(Qt::NoBrush);
            }

            p.drawPath(m_activePreviewPath);
        }
    }

    // 5. Photoshop-Style In-Place Text Editor
    if (m_isEditingText) {
        p.save();
        QFont font;
        font.setPointSize(m_stateMgr->fontSize());
        font.setBold(true);
        p.setFont(font);

        QFontMetrics fm(font);
        const int textW = qMax(40, fm.horizontalAdvance(m_currentEditText));
        const int textH = fm.height();
        const QRectF boxRect(m_textEditPos.x() - 4, m_textEditPos.y() - fm.ascent() - 4, textW + 16, textH + 8);

        // Bounding dashed box
        QPen dashedPen(QColor(56, 189, 248), 1.5, Qt::DashLine);
        p.setPen(dashedPen);
        p.setBrush(QColor(15, 23, 42, 140)); // Translucent editor backdrop
        p.drawRoundedRect(boxRect, 4, 4);

        // Render typed text
        p.setPen(m_stateMgr->currentColor());
        p.drawText(m_textEditPos, m_currentEditText);

        // Blinking cursor
        if (m_cursorVisible) {
            const int cursorX = m_textEditPos.x() + fm.horizontalAdvance(m_currentEditText);
            const int cursorY1 = m_textEditPos.y() - fm.ascent();
            const int cursorY2 = m_textEditPos.y() + fm.descent();
            QPen cursorPen(m_stateMgr->currentColor(), 2.0);
            p.setPen(cursorPen);
            p.drawLine(cursorX + 2, cursorY1, cursorX + 2, cursorY2);
        }

        // Mini Hint pill below box
        QFont hintFont = p.font();
        hintFont.setPointSize(9);
        p.setFont(hintFont);
        p.setPen(QColor(226, 232, 240));
        p.drawText(QPointF(boxRect.x(), boxRect.bottom() + 16), QStringLiteral("[Enter: Tamamla | Esc: İptal]"));

        p.restore();
    }

    // 8. Subtle top badge when in Passthrough mode
    if (m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
        p.save();
        p.setRenderHint(QPainter::Antialiasing, true);

        const QString statusText = QStringLiteral("Masaüstü Modu (Kısayol: F9 / Meta+Alt+D)");
        QFont font = p.font();
        font.setPointSize(10);
        font.setBold(true);
        p.setFont(font);

        QFontMetrics fm(font);
        const int textW = fm.horizontalAdvance(statusText);
        const int badgeW = textW + 30;
        const int badgeH = 28;
        const QRect badgeRect((width() - badgeW) / 2, 10, badgeW, badgeH);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(15, 23, 42, 210)); // Dark sleek badge
        p.drawRoundedRect(badgeRect, 14, 14);

        // Subtle glowing dot
        p.setBrush(QColor(34, 197, 94)); // Emerald green dot
        p.drawEllipse(badgeRect.x() + 12, badgeRect.y() + (badgeH - 8) / 2, 8, 8);

        p.setPen(QColor(241, 245, 249));
        p.drawText(badgeRect.adjusted(26, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, statusText);
        p.restore();
    }
}

void CanvasWindow::tabletEvent(QTabletEvent* ev)
{
    if (m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
        ev->ignore();
        return;
    }

    const QPointF pos = ev->position();
    m_lastMousePos = pos;

    if (!m_toolbarRect.isNull() && m_toolbarRect.contains(pos.toPoint())) {
        ev->ignore();
        return;
    }

    const qreal pressure = ev->pressure();
    const qreal tiltX = ev->xTilt();
    const qreal tiltY = ev->yTilt();

    switch (ev->type()) {
    case QEvent::TabletPress:
        handleStartPoint(pos, pressure, tiltX, tiltY);
        break;
    case QEvent::TabletMove:
        handleMovePoint(pos, pressure, tiltX, tiltY);
        break;
    case QEvent::TabletRelease:
        handleReleasePoint(pos);
        break;
    default:
        break;
    }

    ev->accept();
}

void CanvasWindow::mousePressEvent(QMouseEvent* ev)
{
    if (m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
        ev->ignore();
        return;
    }

    if (ev->source() == Qt::MouseEventSynthesizedByQt) {
        ev->accept();
        return;
    }

    m_lastMousePos = ev->position();

    if (!m_toolbarRect.isNull() && m_toolbarRect.contains(ev->position().toPoint())) {
        ev->ignore();
        return;
    }

    if (ev->button() == Qt::LeftButton) {
        handleStartPoint(ev->position(), 1.0);
        ev->accept();
    }
}

void CanvasWindow::mouseMoveEvent(QMouseEvent* ev)
{
    m_lastMousePos = ev->position();

    if (m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
        ev->ignore();
        return;
    }

    if (ev->source() == Qt::MouseEventSynthesizedByQt) {
        ev->accept();
        return;
    }

    if (m_isDrawing) {
        handleMovePoint(ev->position(), 1.0);
        ev->accept();
    }
}

void CanvasWindow::mouseReleaseEvent(QMouseEvent* ev)
{
    if (m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
        ev->ignore();
        return;
    }

    if (ev->source() == Qt::MouseEventSynthesizedByQt) {
        ev->accept();
        return;
    }

    if (ev->button() == Qt::LeftButton && m_isDrawing) {
        handleReleasePoint(ev->position());
        ev->accept();
    }
}

void CanvasWindow::handleStartPoint(const QPointF& pos, qreal pressure, qreal tiltX, qreal tiltY)
{
    // If currently editing text and clicked elsewhere, commit current text
    if (m_isEditingText) {
        commitCurrentText();
        return;
    }

    const auto tool = m_stateMgr->currentTool();

    // Photoshop-style in-place text tool activation on click
    if (tool == ToolType::Text) {
        m_isEditingText = true;
        m_textEditPos = pos;
        m_currentEditText.clear();
        m_cursorVisible = true;
        m_textCursorTimer.start(500);
        update();
        return;
    }

    m_isDrawing = true;
    m_lastPressure = pressure;
    m_dragStartPos = pos;
    m_activePoints.clear();
    m_activePoints.push_back(StrokePoint(pos, pressure, tiltX, tiltY));

    if (tool == ToolType::Eraser) {
        performVectorErase(pos, m_stateMgr->currentWidth() * 3.0);
    } else if (tool == ToolType::Pen || tool == ToolType::Highlighter || tool == ToolType::GhostPen) {
        m_activePreviewPath = Smoother::buildSmoothPath(m_activePoints);
        update();
    }
}

void CanvasWindow::handleMovePoint(const QPointF& pos, qreal pressure, qreal tiltX, qreal tiltY)
{
    m_lastPressure = pressure;
    m_activePoints.push_back(StrokePoint(pos, pressure, tiltX, tiltY));

    const auto tool = m_stateMgr->currentTool();
    if (tool == ToolType::Eraser) {
        performVectorErase(pos, m_stateMgr->currentWidth() * 3.0);
    } else if (tool == ToolType::Pen || tool == ToolType::Highlighter || tool == ToolType::GhostPen) {
        m_activePreviewPath = Smoother::buildSmoothPath(m_activePoints);
        update();
    } else if (tool == ToolType::Line) {
        m_activePreviewPath = Smoother::buildLinePath(m_dragStartPos, pos);
        update();
    } else if (tool == ToolType::Arrow) {
        m_activePreviewPath = Smoother::buildArrowPath(m_dragStartPos, pos, m_stateMgr->currentWidth() * 4.0);
        update();
    } else if (tool == ToolType::Rectangle) {
        m_activePreviewPath = Smoother::buildRectPath(m_dragStartPos, pos);
        update();
    } else if (tool == ToolType::Ellipse) {
        m_activePreviewPath = Smoother::buildEllipsePath(m_dragStartPos, pos);
        update();
    }
}

void CanvasWindow::handleReleasePoint(const QPointF& pos)
{
    const auto tool = m_stateMgr->currentTool();

    if (tool == ToolType::Eraser) {
        performVectorErase(pos, m_stateMgr->currentWidth() * 3.0);
    } else if (tool == ToolType::GhostPen && m_activePoints.size() >= 1) {
        Stroke s;
        s.tool = ToolType::GhostPen;
        s.color = m_stateMgr->currentColor();
        s.baseWidth = m_stateMgr->currentWidth();
        s.points = m_activePoints;
        s.cachedPath = Smoother::buildSmoothPath(m_activePoints);
        s.cachedBoundingRect = s.cachedPath.boundingRect().adjusted(-s.baseWidth, -s.baseWidth, s.baseWidth, s.baseWidth);
        s.createdAtMs = QDateTime::currentMSecsSinceEpoch();
        s.opacity = 1.0;

        m_ghostStrokes.push_back(s);
        if (!m_ghostTimer.isActive()) {
            m_ghostTimer.start();
        }
    } else if (m_activePoints.size() >= 1) {
        Stroke s;
        s.tool = tool;
        s.color = m_stateMgr->currentColor();
        s.baseWidth = m_stateMgr->currentWidth();
        s.isFilled = m_stateMgr->isFilled();
        s.points = m_activePoints;

        if (tool == ToolType::Pen || tool == ToolType::Highlighter) {
            s.cachedPath = Smoother::buildSmoothPath(m_activePoints);
        } else if (tool == ToolType::Line) {
            s.cachedPath = Smoother::buildLinePath(m_dragStartPos, pos);
        } else if (tool == ToolType::Arrow) {
            s.cachedPath = Smoother::buildArrowPath(m_dragStartPos, pos, m_stateMgr->currentWidth() * 4.0);
        } else if (tool == ToolType::Rectangle) {
            s.cachedPath = Smoother::buildRectPath(m_dragStartPos, pos);
        } else if (tool == ToolType::Ellipse) {
            s.cachedPath = Smoother::buildEllipsePath(m_dragStartPos, pos);
        }

        s.cachedBoundingRect = s.cachedPath.boundingRect().adjusted(-s.baseWidth, -s.baseWidth, s.baseWidth, s.baseWidth);
        s.isDirty = false;

        m_document->addStroke(s);
    }

    m_isDrawing = false;
    m_activePoints.clear();
    m_activePreviewPath = QPainterPath();
    update();
}

void CanvasWindow::performVectorErase(const QPointF& pos, qreal radius)
{
    QVector<QUuid> toErase;
    for (const auto& stroke : m_document->strokes()) {
        if (Smoother::strokeIntersects(stroke, pos, radius)) {
            toErase.push_back(stroke.id);
        }
    }

    if (!toErase.isEmpty()) {
        m_document->removeStrokes(toErase);
    }
}

void CanvasWindow::keyPressEvent(QKeyEvent* ev)
{
    // Handle Photoshop-style in-place text typing
    if (m_isEditingText) {
        if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
            commitCurrentText();
            ev->accept();
            return;
        } else if (ev->key() == Qt::Key_Escape) {
            cancelCurrentText();
            ev->accept();
            return;
        } else if (ev->key() == Qt::Key_Backspace) {
            if (!m_currentEditText.isEmpty()) {
                m_currentEditText.chop(1);
                update();
            }
            ev->accept();
            return;
        } else if (!ev->text().isEmpty()) {
            const QString txt = ev->text();
            if (txt[0].isPrint()) {
                m_currentEditText += txt;
                m_cursorVisible = true;
                update();
            }
            ev->accept();
            return;
        }
    }

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

    if (ev->key() == Qt::Key_Escape) {
        if (m_isDrawing) {
            m_isDrawing = false;
            m_activePoints.clear();
            m_activePreviewPath = QPainterPath();
            update();
        } else {
            m_stateMgr->setInteractionMode(InteractionMode::DesktopPassthrough);
        }
        ev->accept();
        return;
    }

    if (ev->modifiers() & Qt::ControlModifier) {
        if (ev->key() == Qt::Key_Z) {
            if (ev->modifiers() & Qt::ShiftModifier) {
                m_document->redo();
            } else {
                m_document->undo();
            }
            ev->accept();
            return;
        } else if (ev->key() == Qt::Key_Y) {
            m_document->redo();
            ev->accept();
            return;
        } else if ((ev->modifiers() & Qt::ShiftModifier) && ev->key() == Qt::Key_C) {
            m_document->clear();
            ev->accept();
            return;
        } else if ((ev->modifiers() & Qt::ShiftModifier) && ev->key() == Qt::Key_S) {
            m_stateMgr->screenshotRequested();
            ev->accept();
            return;
        }
    }

    if (ev->key() == Qt::Key_C && !(ev->modifiers() & Qt::ControlModifier)) {
        m_document->clear();
        ev->accept();
        return;
    }

    QRasterWindow::keyPressEvent(ev);
}

} // namespace DrawOnScreen
