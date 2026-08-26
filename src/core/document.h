#pragma once

#include "types.h"
#include <QObject>
#include <QVector>
#include <memory>

namespace DrawOnScreen {

class CanvasDocument;

class CanvasCommand {
public:
    virtual ~CanvasCommand() = default;
    virtual void undo(CanvasDocument* doc) = 0;
    virtual void redo(CanvasDocument* doc) = 0;
};

class CanvasDocument : public QObject {
    Q_OBJECT
public:
    explicit CanvasDocument(QObject* parent = nullptr);

    const QVector<Stroke>& strokes() const { return m_strokes; }
    int strokeCount() const { return m_strokes.size(); }

    void addStroke(const Stroke& stroke);
    void removeStrokeById(const QUuid& id);
    void removeStrokes(const QVector<QUuid>& ids);
    void clear();

    void undo();
    void redo();
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }

    void render(QPainter& painter, const QRectF& clipRect = QRectF()) const;

    // Internal methods for commands
    void internalAddStroke(const Stroke& stroke, int index = -1);
    void internalRemoveStroke(const QUuid& id);
    void internalRestoreStrokes(const QVector<QPair<int, Stroke>>& indexedStrokes);
    void internalClearStrokes();

signals:
    void documentChanged();
    void undoRedoAvailabilityChanged();

private:
    void pushCommand(std::shared_ptr<CanvasCommand> command);

    QVector<Stroke> m_strokes;
    QVector<std::shared_ptr<CanvasCommand>> m_undoStack;
    QVector<std::shared_ptr<CanvasCommand>> m_redoStack;
    static constexpr int MaxUndoDepth = 100;
};

} // namespace DrawOnScreen
