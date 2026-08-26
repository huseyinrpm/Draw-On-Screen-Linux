import QtQuick
import QtQuick.Controls

Item {
    id: root

    property bool isVertical: toolState.isVerticalLayout
    property real uiScale: toolState ? toolState.uiScale : 1.0

    implicitWidth: colorGrid.width
    implicitHeight: colorGrid.height

    property var colors: [
        "#ef4444", // Red
        "#f97316", // Orange
        "#eab308", // Yellow
        "#10b981", // Green
        "#06b6d4", // Cyan
        "#3b82f6", // Blue
        "#a855f7", // Purple
        "#ec4899", // Pink
        "#ffffff", // White
        "#000000"  // Black
    ]

    Grid {
        id: colorGrid
        columns: root.isVertical ? 2 : 10
        rows: root.isVertical ? 5 : 1
        spacing: Math.round(5 * root.uiScale)
        anchors.centerIn: parent

        Repeater {
            model: root.colors
            delegate: Rectangle {
                id: colorItem
                width: Math.round(18 * root.uiScale)
                height: Math.round(18 * root.uiScale)
                radius: Math.round(9 * root.uiScale)
                color: modelData

                property bool isSelected: toolState.currentColor.toString().toLowerCase() === modelData.toLowerCase()

                border.color: isSelected ? "#ffffff" : (modelData === "#000000" ? "#475569" : "transparent")
                border.width: isSelected ? 2 : (modelData === "#000000" ? 1 : 0)

                scale: isSelected ? 1.25 : (mouseArea.containsMouse ? 1.15 : 1.0)
                Behavior on scale { NumberAnimation { duration: 120 } }

                Rectangle {
                    visible: colorItem.isSelected
                    anchors.centerIn: parent
                    width: Math.max(3, Math.round(5 * root.uiScale))
                    height: Math.max(3, Math.round(5 * root.uiScale))
                    radius: width / 2
                    color: modelData === "#ffffff" ? "#0f172a" : "#ffffff"
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        toolState.setInteractionModeEnum(0)
                        toolState.selectColor(modelData)
                    }
                }
            }
        }
    }
}
