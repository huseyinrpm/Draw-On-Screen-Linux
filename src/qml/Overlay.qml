import QtQuick
import QtQuick.Controls
import DrawOnScreen 1.0

Item {
    id: root
    anchors.fill: parent

    // 1. Full-Screen Drawing Canvas
    CanvasItem {
        id: canvas
        anchors.fill: parent
        document: canvasDoc
        stateManager: toolState
        toolbarRect: Qt.rect(toolbarCard.x, toolbarCard.y, toolbarCard.width, toolbarCard.height)
        focus: true
    }

    // 2. Floating Toolbar Container with 0ms Instant Dragging & Position Persistence
    Item {
        id: toolbarCard
        x: toolState.savedToolbarX
        y: toolState.savedToolbarY
        width: toolbarComponent.width
        height: toolbarComponent.height
        z: 10

        onXChanged: overlayWindow.setToolbarRect(Qt.rect(x, y, width, height))
        onYChanged: overlayWindow.setToolbarRect(Qt.rect(x, y, width, height))
        onWidthChanged: overlayWindow.setToolbarRect(Qt.rect(x, y, width, height))
        onHeightChanged: overlayWindow.setToolbarRect(Qt.rect(x, y, width, height))

        Toolbar {
            id: toolbarComponent

            onMoveRequested: (dx, dy) => {
                var newX = toolbarCard.x + dx;
                var newY = toolbarCard.y + dy;
                toolbarCard.x = Math.max(0, Math.min(root.width - toolbarCard.width, newX));
                toolbarCard.y = Math.max(0, Math.min(root.height - toolbarCard.height, newY));
            }

            onDragFinished: {
                toolState.saveToolbarPosition(toolbarCard.x, toolbarCard.y);
            }
        }
    }
}
