#include "canvasitem.h"
#include "../core/smoother.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QDateTime>
#include <algorithm>

namespace DrawOnScreen {

CanvasItem::CanvasItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAntialiasing(true);
    setOpaquePainting(false);
    setSmooth(true);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    connect(&m_textCursorTimer, &QTimer::timeout, this, &CanvasItem::onCursorBlink);
    connect(&m_ghostTimer, &QTimer::timeout, this, &CanvasItem::onGhostTick);
    m_ghostTimer.setInterval(1000 / 60); // 60 FPS smooth fade
}

CanvasItem::~CanvasItem() = default;

void CanvasItem::setDocument(CanvasDocument* doc)
{
    if (m_document != doc) {
        if (m_document) {
            disconnect(m_document, nullptr, this, nullptr);
        }
        m_document = doc;
        if (m_document) {
            connect(m_document, &CanvasDocument::documentChanged, this, [this]() { update(); });
        }
        emit documentChanged();
        update();
    }
}

void CanvasItem::setStateManager(ToolStateManager* mgr)
{
    if (m_stateMgr != mgr) {
        if (m_stateMgr) {
            disconnect(m_stateMgr, nullptr, this, nullptr);
        }
        m_stateMgr = mgr;
        if (m_stateMgr) {
            connect(m_stateMgr, &ToolStateManager::backgroundModeChanged, this, [this](int) { update(); });
            connect(m_stateMgr, &ToolStateManager::areDrawingsVisibleChanged, this, [this](bool) { update(); });
            connect(m_stateMgr, &ToolStateManager::currentToolChanged, this, [this](int) { update(); });
        }
        emit stateManagerChanged();
        update();
    }
}

void CanvasItem::setToolbarRect(const QRect& rect)
{
    if (m_toolbarRect != rect) {
        m_toolbarRect = rect;
        emit toolbarRectChanged();
        update();
    }
}

void CanvasItem::onCursorBlink()
{
    m_cursorVisible = !m_cursorVisible;
    if (m_isEditingText) {
        update();
    }
}

void CanvasItem::onGhostTick()
{
    if (m_ghostStrokes.isEmpty()) {
        m_ghostTimer.stop();
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 lifetimeMs = m_stateMgr ? m_stateMgr->ghostDurationMs() : 3000;
    const qint64 fadeStartTimeMs = static_cast<qint64>(lifetimeMs * 0.6);
    const qint64 fadeDurationMs = lifetimeMs - fadeStartTimeMs;

    for (auto& s : m_ghostStrokes) {
        const qint64 elapsed = now - s.createdAtMs;
        if (elapsed < fadeStartTimeMs) {
            s.opacity = 1.0;
        } else {
            const qreal progress = static_cast<qreal>(elapsed - fadeStartTimeMs) / fadeDurationMs;
            s.opacity = qMax(0.0, 1.0 - progress);
        }
    }

    m_ghostStrokes.erase(
        std::remove_if(m_ghostStrokes.begin(), m_ghostStrokes.end(), [](const Stroke& s) {
            return s.opacity <= 0.01;
        }),
        m_ghostStrokes.end()
    );

    update();
}

void CanvasItem::commitCurrentText()
{
    if (!m_isEditingText) return;

    if (!m_currentEditText.isEmpty() && m_document && m_stateMgr) {
        Stroke s;
        s.tool = ToolType::Text;
        s.color = m_stateMgr->currentColor();
        s.text = m_currentEditText;
        QFont font;
        font.setPointSize(m_stateMgr->fontSize());
        font.setBold(true);
        s.font = font;

        QFontMetrics fm(font);
        QRectF bounds = fm.boundingRect(s.text);
        bounds.moveTo(m_textEditPos.x(), m_textEditPos.y() - fm.ascent());
        s.cachedBoundingRect = bounds;

        s.points.push_back(StrokePoint(m_textEditPos));
        m_document->addStroke(s);
    }

    m_isEditingText = false;
    m_currentEditText.clear();
    m_textCursorTimer.stop();
    update();
}

void CanvasItem::cancelCurrentText()
{
    m_isEditingText = false;
    m_currentEditText.clear();
    m_textCursorTimer.stop();
    update();
}

void CanvasItem::paint(QPainter* painter)
{
    if (!painter || !m_stateMgr || !m_document) return;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF itemRect(0, 0, width(), height());

    // 1. Background mode (Transparent / Whiteboard / Blackboard) - hidden when drawings are toggled off
    if (m_stateMgr->areDrawingsVisible()) {
        const auto bgMode = m_stateMgr->backgroundMode();
        if (bgMode == BackgroundMode::Whiteboard) {
            painter->fillRect(itemRect, QColor(248, 250, 252));
        } else if (bgMode == BackgroundMode::Blackboard) {
            painter->fillRect(itemRect, QColor(15, 23, 42));
        }
    }

    // 2. Render committed strokes if visible
    if (m_stateMgr->areDrawingsVisible()) {
        m_document->render(*painter, itemRect);
    }

    // 3. Render Ghost Pen Fading Strokes
    for (const auto& s : m_ghostStrokes) {
        painter->save();
        QColor ghostCol = s.color;
        ghostCol.setAlphaF(ghostCol.alphaF() * s.opacity);
        QPen pen(ghostCol, s.baseWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(s.cachedPath);
        painter->restore();
    }

    // 4. Render active in-progress stroke / shape preview
    if (m_isDrawing && m_stateMgr->currentTool() != ToolType::Eraser) {
        const auto tool = m_stateMgr->currentTool();
        const QColor col = m_stateMgr->currentColor();
        const qreal baseW = m_stateMgr->currentWidth();

        if (tool == ToolType::Highlighter) {
            painter->save();
            QColor hl = col;
            hl.setAlphaF(0.40);
            QPen pen(hl, baseW, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(m_activePreviewPath);
            painter->restore();
        } else if (tool == ToolType::GhostPen) {
            QPen pen(col, baseW * qMax(0.2, m_lastPressure), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(m_activePreviewPath);
        } else {
            QPen pen(col, baseW * (tool == ToolType::Pen ? qMax(0.2, m_lastPressure) : 1.0),
                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(pen);

            if (m_stateMgr->isFilled() && (tool == ToolType::Rectangle || tool == ToolType::Ellipse)) {
                QColor fill = col;
                fill.setAlphaF(0.25);
                painter->setBrush(fill);
            } else {
                painter->setBrush(Qt::NoBrush);
            }

            painter->drawPath(m_activePreviewPath);
        }
    }

    // 5. Photoshop-Style In-Place Text Editor
    if (m_isEditingText) {
        painter->save();
        QFont font;
        font.setPointSize(m_stateMgr->fontSize());
        font.setBold(true);
        painter->setFont(font);

        QFontMetrics fm(font);
        const int fontHeight = fm.height();
        const int fontAscent = fm.ascent();
        const int textWidth = fm.horizontalAdvance(m_currentEditText.isEmpty() ? QStringLiteral(" ") : m_currentEditText);

        QRectF textRect(m_textEditPos.x() - 4, m_textEditPos.y() - fontAscent - 4, qMax(textWidth + 24, 120), fontHeight + 8);

        // Semi-transparent backdrop card
        painter->fillRect(textRect, QColor(15, 23, 42, 220));

        // Dashed border like Photoshop/Illustrator
        QPen borderPen(QColor(56, 189, 248), 1.5, Qt::DashLine);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(textRect, 4, 4);

        // Draw current text
        painter->setPen(m_stateMgr->currentColor());
        painter->drawText(m_textEditPos, m_currentEditText);

        // Blinking Text Cursor
        if (m_cursorVisible) {
            int cursorX = m_textEditPos.x() + fm.horizontalAdvance(m_currentEditText) + 2;
            QPen cursorPen(QColor(248, 250, 252), 2);
            painter->setPen(cursorPen);
            painter->drawLine(cursorX, m_textEditPos.y() - fontAscent, cursorX, m_textEditPos.y() + (fontHeight - fontAscent));
        }

        painter->restore();
    }
}

void CanvasItem::keyPressEvent(QKeyEvent* ev)
{
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
        } else {
            const QString text = ev->text();
            if (!text.isEmpty() && text.at(0).isPrint()) {
                m_currentEditText.append(text);
                m_cursorVisible = true;
                update();
                ev->accept();
                return;
            }
        }
    }

    QQuickPaintedItem::keyPressEvent(ev);
}

void CanvasItem::mousePressEvent(QMouseEvent* ev)
{
    if (!m_stateMgr || m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
        ev->ignore();
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

void CanvasItem::mouseMoveEvent(QMouseEvent* ev)
{
    m_lastMousePos = ev->position();

    if (!m_stateMgr || m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
        ev->ignore();
        return;
    }

    if (m_isDrawing) {
        handleMovePoint(ev->position(), 1.0);
        ev->accept();
    }
}

void CanvasItem::mouseReleaseEvent(QMouseEvent* ev)
{
    if (!m_stateMgr || m_stateMgr->interactionMode() == InteractionMode::DesktopPassthrough) {
        ev->ignore();
        return;
    }

    if (ev->button() == Qt::LeftButton && m_isDrawing) {
        handleReleasePoint(ev->position());
        ev->accept();
    }
}

void CanvasItem::handleStartPoint(const QPointF& pos, qreal pressure, qreal tiltX, qreal tiltY)
{
    if (m_isEditingText) {
        commitCurrentText();
        return;
    }

    const auto tool = m_stateMgr->currentTool();

    if (tool == ToolType::Text) {
        m_isEditingText = true;
        m_textEditPos = pos;
        m_currentEditText.clear();
        m_cursorVisible = true;
        m_textCursorTimer.start(500);
        forceActiveFocus();
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

void CanvasItem::handleMovePoint(const QPointF& pos, qreal pressure, qreal tiltX, qreal tiltY)
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

void CanvasItem::handleReleasePoint(const QPointF& pos)
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
    } else if (m_activePoints.size() >= 1 && m_document) {
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
        m_document->addStroke(s);
    }

    m_isDrawing = false;
    m_activePoints.clear();
    m_activePreviewPath = QPainterPath();
    update();
}

void CanvasItem::performVectorErase(const QPointF& pos, qreal radius)
{
    if (!m_document) return;

    QRectF eraseRect(pos.x() - radius, pos.y() - radius, radius * 2, radius * 2);
    const auto& strokes = m_document->strokes();
    QVector<QUuid> toRemove;

    for (const auto& s : strokes) {
        if (s.cachedBoundingRect.intersects(eraseRect)) {
            toRemove.push_back(s.id);
        }
    }

    for (const auto& id : toRemove) {
        m_document->removeStrokeById(id);
    }

    m_ghostStrokes.erase(
        std::remove_if(m_ghostStrokes.begin(), m_ghostStrokes.end(), [&](const Stroke& s) {
            return s.cachedBoundingRect.intersects(eraseRect);
        }),
        m_ghostStrokes.end()
    );

    update();
}

} // namespace DrawOnScreen
