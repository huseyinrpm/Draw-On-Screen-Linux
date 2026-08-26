#include <QCoreApplication>
#include <cassert>
#include <iostream>

#include "core/types.h"
#include "core/smoother.h"
#include "core/document.h"
#include "core/laserengine.h"
#include "core/toolstatemanager.h"

using namespace DrawOnScreen;

void testSmoother()
{
    std::cout << "[TEST] Running Smoother tests..." << std::endl;

    // Test 1: Empty and single point smoothing
    QVector<StrokePoint> emptyPts;
    QPainterPath emptyPath = Smoother::buildSmoothPath(emptyPts);
    assert(emptyPath.isEmpty());

    QVector<StrokePoint> singlePts = { StrokePoint(QPointF(10, 10)) };
    QPainterPath singlePath = Smoother::buildSmoothPath(singlePts);
    assert(!singlePath.isEmpty());

    // Test 2: Multi-point Catmull-Rom smoothing
    QVector<StrokePoint> multiPts = {
        StrokePoint(QPointF(0, 0)),
        StrokePoint(QPointF(10, 20)),
        StrokePoint(QPointF(20, 10)),
        StrokePoint(QPointF(30, 30))
    };
    QPainterPath smoothPath = Smoother::buildSmoothPath(multiPts);
    assert(!smoothPath.isEmpty());
    assert(smoothPath.elementCount() >= 4);

    // Test 3: Distance to segment
    qreal dist1 = Smoother::distanceToSegment(QPointF(5, 5), QPointF(0, 0), QPointF(10, 0));
    assert(std::abs(dist1 - 5.0) < 0.001);

    qreal dist2 = Smoother::distanceToSegment(QPointF(15, 0), QPointF(0, 0), QPointF(10, 0));
    assert(std::abs(dist2 - 5.0) < 0.001);

    // Test 4: Stroke collision for vector eraser
    Stroke stroke;
    stroke.tool = ToolType::Pen;
    stroke.baseWidth = 4.0;
    stroke.points = multiPts;
    stroke.cachedPath = smoothPath;
    stroke.cachedBoundingRect = smoothPath.boundingRect();

    assert(Smoother::strokeIntersects(stroke, QPointF(10, 20), 5.0));
    assert(!Smoother::strokeIntersects(stroke, QPointF(100, 100), 5.0));

    std::cout << "[PASS] Smoother tests passed." << std::endl;
}

void testDocument()
{
    std::cout << "[TEST] Running CanvasDocument tests..." << std::endl;

    CanvasDocument doc;
    assert(doc.strokeCount() == 0);
    assert(!doc.canUndo());
    assert(!doc.canRedo());

    // Add Stroke
    Stroke s1;
    s1.tool = ToolType::Pen;
    s1.color = Qt::red;
    s1.points = { StrokePoint(QPointF(0, 0)), StrokePoint(QPointF(10, 10)) };
    doc.addStroke(s1);

    assert(doc.strokeCount() == 1);
    assert(doc.canUndo());
    assert(!doc.canRedo());

    // Add second Stroke
    Stroke s2;
    s2.tool = ToolType::Highlighter;
    s2.color = Qt::yellow;
    s2.points = { StrokePoint(QPointF(20, 20)), StrokePoint(QPointF(30, 30)) };
    doc.addStroke(s2);

    assert(doc.strokeCount() == 2);

    // Test Undo
    doc.undo();
    assert(doc.strokeCount() == 1);
    assert(doc.canUndo());
    assert(doc.canRedo());

    // Test Redo
    doc.redo();
    assert(doc.strokeCount() == 2);
    assert(!doc.canRedo());

    // Test Clear and Undo-Clear
    doc.clear();
    assert(doc.strokeCount() == 0);
    assert(doc.canUndo());

    doc.undo();
    assert(doc.strokeCount() == 2);

    // Test Erase by ID
    doc.removeStrokeById(s1.id);
    assert(doc.strokeCount() == 1);
    assert(doc.strokes()[0].id == s2.id);

    doc.undo();
    assert(doc.strokeCount() == 2);

    std::cout << "[PASS] CanvasDocument tests passed." << std::endl;
}

void testToolStateManager()
{
    std::cout << "[TEST] Running ToolStateManager tests..." << std::endl;

    ToolStateManager state;
    state.setCurrentTool(ToolType::Pen);
    assert(state.currentTool() == ToolType::Pen);

    state.setInteractionMode(InteractionMode::Drawing);
    assert(state.interactionMode() == InteractionMode::Drawing);

    state.setCurrentTool(ToolType::Laser);
    assert(state.currentTool() == ToolType::Laser);
    assert(state.isLaserActive());

    state.toggleInteractionMode();
    assert(state.interactionMode() == InteractionMode::DesktopPassthrough);

    state.setCurrentWidth(12.5);
    assert(std::abs(state.currentWidth() - 12.5) < 0.001);

    state.setCurrentTool(ToolType::GhostPen);
    assert(state.currentTool() == ToolType::GhostPen);

    state.setBackgroundMode(BackgroundMode::Transparent);
    state.cycleBackgroundMode();
    assert(state.backgroundMode() == BackgroundMode::Whiteboard);

    state.setIsVerticalLayout(false);
    state.toggleOrientation();
    assert(state.isVerticalLayout());

    state.setShortcutToggleMode(QStringLiteral("F10"));
    assert(state.shortcutToggleMode() == QStringLiteral("F10"));

    state.setCustomShortcut(QStringLiteral("undo"), QStringLiteral("Ctrl+Alt+Z"));
    assert(state.shortcutUndo() == QStringLiteral("Ctrl+Alt+Z"));

    state.resetShortcutsToDefaults();
    assert(state.shortcutToggleMode() == QStringLiteral("F9"));
    assert(state.shortcutUndo() == QStringLiteral("Ctrl+Z"));

    std::cout << "[PASS] ToolStateManager tests passed." << std::endl;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    testSmoother();
    testDocument();
    testToolStateManager();

    std::cout << "All DrawOnScreen unit tests passed successfully!" << std::endl;
    return 0;
}
