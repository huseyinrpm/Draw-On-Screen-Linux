import QtQuick
import QtQuick.Controls

Item {
    id: root
    property real uiScale: toolState ? toolState.uiScale : 1.0

    width: Math.round(140 * uiScale)
    height: Math.round(38 * uiScale)

    Row {
        anchors.fill: parent
        spacing: Math.round(8 * root.uiScale)
        anchors.verticalCenter: parent.verticalCenter

        // Live preview circle
        Rectangle {
            id: previewCircle
            width: Math.round(24 * root.uiScale)
            height: Math.round(24 * root.uiScale)
            radius: width / 2
            color: "transparent"
            border.color: Qt.rgba(255, 255, 255, 0.2)
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter

            Rectangle {
                anchors.centerIn: parent
                width: Math.max(2, Math.min(20, toolState.currentWidth))
                height: width
                radius: width / 2
                color: toolState.currentColor
            }
        }

        // Custom stylized slider
        Slider {
            id: slider
            from: 1
            to: 36
            stepSize: 1
            value: toolState.currentWidth
            anchors.verticalCenter: parent.verticalCenter
            width: Math.round(100 * root.uiScale)

            onMoved: {
                toolState.selectWidth(value)
            }

            background: Rectangle {
                x: slider.leftPadding
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: 100
                implicitHeight: 4
                width: slider.availableWidth
                height: implicitHeight
                radius: 2
                color: Qt.rgba(255, 255, 255, 0.15)

                Rectangle {
                    width: slider.visualPosition * parent.width
                    height: parent.height
                    color: "#38bdf8"
                    radius: 2
                }
            }

            handle: Rectangle {
                x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: 14
                implicitHeight: 14
                radius: 7
                color: "#ffffff"
                border.color: "#38bdf8"
                border.width: 2
            }
        }
    }
}
