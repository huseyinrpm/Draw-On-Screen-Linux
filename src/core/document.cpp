#include "document.h"
#include "smoother.h"
#include <QPainter>

namespace DrawOnScreen {

class AddStrokeCommand : public CanvasCommand {
public:
    explicit AddStrokeCommand(const Stroke& stroke) : m_stroke(stroke) {}

    void undo(CanvasDocument* doc) override {
        doc->internalRemoveStroke(m_stroke.id);
    }

    void redo(CanvasDocument* doc) override {
        doc->internalAddStroke(m_stroke);
    }

private:
    Stroke m_stroke;
};

class EraseStrokesCommand : public CanvasCommand {
public:
    explicit EraseStrokesCommand(const QVector<QPair<int, Stroke>>& removed)
        : m_removed(removed) {}

    void undo(CanvasDocument* doc) override {
        doc->internalRestoreStrokes(m_removed);
    }

    void redo(CanvasDocument* doc) override {
        for (const auto& pair : m_removed) {
            doc->internalRemoveStroke(pair.second.id);
        }
    }

private:
    QVector<QPair<int, Stroke>> m_removed;
};

class ClearCommand : public CanvasCommand {
public:
    explicit ClearCommand(const QVector<Stroke>& strokes) : m_strokes(strokes) {}

    void undo(CanvasDocument* doc) override {
        for (const auto& stroke : m_strokes) {
            doc->internalAddStroke(stroke);
        }
    }

    void redo(CanvasDocument* doc) override {
        doc->internalClearStrokes();
    }

private:
    QVector<Stroke> m_strokes;
};

CanvasDocument::CanvasDocument(QObject* parent)
    : QObject(parent)
{
}

void CanvasDocument::addStroke(const Stroke& stroke)
{
    m_strokes.push_back(stroke);
    pushCommand(std::make_shared<AddStrokeCommand>(stroke));
    emit documentChanged();
}

void CanvasDocument::removeStrokeById(const QUuid& id)
{
    removeStrokes({id});
}

void CanvasDocument::removeStrokes(const QVector<QUuid>& ids)
{
    if (ids.isEmpty()) return;

    QVector<QPair<int, Stroke>> removed;
    for (const auto& id : ids) {
        for (int i = 0; i < m_strokes.size(); ++i) {
            if (m_strokes[i].id == id) {
                removed.push_back({i, m_strokes[i]});
                m_strokes.removeAt(i);
                break;
            }
        }
    }

    if (!removed.isEmpty()) {
        pushCommand(std::make_shared<EraseStrokesCommand>(removed));
        emit documentChanged();
    }
}

void CanvasDocument::clear()
{
    if (m_strokes.isEmpty()) return;

    pushCommand(std::make_shared<ClearCommand>(m_strokes));
    m_strokes.clear();
    emit documentChanged();
}

void CanvasDocument::undo()
{
    if (m_undoStack.isEmpty()) return;

    auto cmd = m_undoStack.takeLast();
    cmd->undo(this);
    m_redoStack.push_back(cmd);

    emit undoRedoAvailabilityChanged();
    emit documentChanged();
}

void CanvasDocument::redo()
{
    if (m_redoStack.isEmpty()) return;

    auto cmd = m_redoStack.takeLast();
    cmd->redo(this);
    m_undoStack.push_back(cmd);

    emit undoRedoAvailabilityChanged();
    emit documentChanged();
}

void CanvasDocument::pushCommand(std::shared_ptr<CanvasCommand> command)
{
    m_undoStack.push_back(command);
    if (m_undoStack.size() > MaxUndoDepth) {
        m_undoStack.removeFirst();
    }
    m_redoStack.clear();
    emit undoRedoAvailabilityChanged();
}

void CanvasDocument::internalAddStroke(const Stroke& stroke, int index)
{
    if (index >= 0 && index <= m_strokes.size()) {
        m_strokes.insert(index, stroke);
    } else {
        m_strokes.push_back(stroke);
    }
}

void CanvasDocument::internalRemoveStroke(const QUuid& id)
{
    for (int i = 0; i < m_strokes.size(); ++i) {
        if (m_strokes[i].id == id) {
            m_strokes.removeAt(i);
            break;
        }
    }
}

void CanvasDocument::internalRestoreStrokes(const QVector<QPair<int, Stroke>>& indexedStrokes)
{
    for (const auto& pair : indexedStrokes) {
        int idx = pair.first;
        if (idx >= 0 && idx <= m_strokes.size()) {
            m_strokes.insert(idx, pair.second);
        } else {
            m_strokes.push_back(pair.second);
        }
    }
}

void CanvasDocument::internalClearStrokes()
{
    m_strokes.clear();
}

void CanvasDocument::render(QPainter& painter, const QRectF& clipRect) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    for (const auto& stroke : m_strokes) {
        // Fast bounding rect culling if clip rect is given
        if (!clipRect.isNull() && !stroke.cachedBoundingRect.isNull()) {
            if (!stroke.cachedBoundingRect.intersects(clipRect)) {
                continue;
            }
        }

        if (stroke.tool == ToolType::Text) {
            painter.setFont(stroke.font);
            painter.setPen(stroke.color);
            if (!stroke.points.isEmpty()) {
                painter.drawText(stroke.points[0].pos, stroke.text);
            }
            continue;
        }

        if (stroke.tool == ToolType::Highlighter) {
            painter.save();
            painter.setCompositionMode(QPainter::CompositionMode_Multiply);
            QColor hlColor = stroke.color;
            hlColor.setAlphaF(0.40);
            QPen pen(hlColor, stroke.baseWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(stroke.cachedPath);
            painter.restore();
            continue;
        }

        QPen pen(stroke.color, stroke.baseWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);

        if (stroke.isFilled && (stroke.tool == ToolType::Rectangle || stroke.tool == ToolType::Ellipse)) {
            QColor fillCol = stroke.color;
            fillCol.setAlphaF(0.25);
            painter.setBrush(fillCol);
        } else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawPath(stroke.cachedPath);
    }

    painter.restore();
}

} // namespace DrawOnScreen
