import QtQuick
import QtQuick.Controls

Item {
    id: root

    property string iconName: ""
    property string tooltipText: ""
    property bool isActive: false
    property bool isDisabled: false
    property color activeColor: "#38bdf8"
    property color customColor: "transparent"
    property bool isCustomColorBadge: false

    signal clicked()

    width: 38
    height: 38

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 10
        color: {
            if (root.isDisabled) return "transparent"
            if (root.isActive) return Qt.rgba(56/255, 189/255, 248/255, 0.22)
            if (mouseArea.containsPress) return Qt.rgba(255, 255, 255, 0.18)
            if (mouseArea.containsMouse) return Qt.rgba(255, 255, 255, 0.10)
            return "transparent"
        }

        border.color: {
            if (root.isActive) return root.activeColor
            if (mouseArea.containsMouse && !root.isDisabled) return Qt.rgba(255, 255, 255, 0.25)
            return "transparent"
        }
        border.width: root.isActive ? 1.5 : 1

        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on border.color { ColorAnimation { duration: 150 } }

        // Glow indicator for active state
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2
            anchors.horizontalCenter: parent.horizontalCenter
            width: root.isActive ? 14 : 0
            height: 2
            radius: 1
            color: root.activeColor
            visible: root.isActive

            Behavior on width { NumberAnimation { duration: 180; easing.type: Easing.OutQuad } }
        }

        // Custom color swatch badge
        Rectangle {
            id: colorSwatch
            visible: root.isCustomColorBadge
            anchors.centerIn: parent
            width: 22
            height: 22
            radius: 11
            color: root.customColor
            border.color: "#ffffff"
            border.width: 1.5
        }

        // Vector icon canvas
        Canvas {
            id: iconCanvas
            visible: !root.isCustomColorBadge
            anchors.fill: parent
            anchors.margins: 9
            renderTarget: Canvas.FramebufferObject

            property color iconColor: {
                if (root.isDisabled) return "#64748b"
                if (root.isActive) return root.activeColor
                if (mouseArea.containsMouse) return "#f8fafc"
                return "#cbd5e1"
            }

            onIconColorChanged: requestPaint()

            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();
                var w = width;
                var h = height;

                ctx.strokeStyle = iconColor;
                ctx.fillStyle = iconColor;
                ctx.lineWidth = 1.8;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";

                if (root.iconName === "mouse") {
                    // Cursor pointer icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.15, h * 0.1);
                    ctx.lineTo(w * 0.15, h * 0.85);
                    ctx.lineTo(w * 0.45, h * 0.65);
                    ctx.lineTo(w * 0.75, h * 0.95);
                    ctx.lineTo(w * 0.9, h * 0.8);
                    ctx.lineTo(w * 0.55, h * 0.55);
                    ctx.lineTo(w * 0.85, h * 0.55);
                    ctx.closePath();
                    ctx.stroke();
                } else if (root.iconName === "pen") {
                    // Pen / pencil icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.7, h * 0.1);
                    ctx.lineTo(w * 0.9, h * 0.3);
                    ctx.lineTo(w * 0.35, h * 0.85);
                    ctx.lineTo(w * 0.1, h * 0.9);
                    ctx.lineTo(w * 0.15, h * 0.65);
                    ctx.closePath();
                    ctx.stroke();
                } else if (root.iconName === "highlighter") {
                    // Highlighter icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.65, h * 0.1);
                    ctx.lineTo(w * 0.9, h * 0.35);
                    ctx.lineTo(w * 0.5, h * 0.75);
                    ctx.lineTo(w * 0.25, h * 0.5);
                    ctx.closePath();
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.moveTo(w * 0.35, h * 0.65);
                    ctx.lineTo(w * 0.1, h * 0.9);
                    ctx.stroke();
                } else if (root.iconName === "laser") {
                    // Laser pointer beam icon
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.5, 4, 0, 2 * Math.PI);
                    ctx.fill();
                    // Laser rays
                    ctx.beginPath();
                    ctx.moveTo(w * 0.5, h * 0.1); ctx.lineTo(w * 0.5, h * 0.25);
                    ctx.moveTo(w * 0.5, h * 0.75); ctx.lineTo(w * 0.5, h * 0.9);
                    ctx.moveTo(w * 0.1, h * 0.5); ctx.lineTo(w * 0.25, h * 0.5);
                    ctx.moveTo(w * 0.75, h * 0.5); ctx.lineTo(w * 0.9, h * 0.5);
                    ctx.stroke();
                } else if (root.iconName === "eraser") {
                    // Eraser icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.5, h * 0.1);
                    ctx.lineTo(w * 0.9, h * 0.5);
                    ctx.lineTo(w * 0.55, h * 0.85);
                    ctx.lineTo(w * 0.15, h * 0.45);
                    ctx.closePath();
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.moveTo(w * 0.35, h * 0.65);
                    ctx.lineTo(w * 0.7, h * 0.3);
                    ctx.stroke();
                } else if (root.iconName === "line") {
                    // Straight line icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.15, h * 0.85);
                    ctx.lineTo(w * 0.85, h * 0.15);
                    ctx.stroke();
                } else if (root.iconName === "arrow") {
                    // Arrow icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.15, h * 0.85);
                    ctx.lineTo(w * 0.85, h * 0.15);
                    ctx.moveTo(w * 0.5, h * 0.15);
                    ctx.lineTo(w * 0.85, h * 0.15);
                    ctx.lineTo(w * 0.85, h * 0.5);
                    ctx.stroke();
                } else if (root.iconName === "rect") {
                    // Rectangle icon
                    ctx.beginPath();
                    ctx.roundRect(w * 0.15, h * 0.2, w * 0.7, h * 0.6, 2);
                    ctx.stroke();
                } else if (root.iconName === "ellipse") {
                    // Circle icon
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.5, w * 0.35, 0, 2 * Math.PI);
                    ctx.stroke();
                } else if (root.iconName === "text") {
                    // Text 'T' icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.2, h * 0.2);
                    ctx.lineTo(w * 0.8, h * 0.2);
                    ctx.moveTo(w * 0.5, h * 0.2);
                    ctx.lineTo(w * 0.5, h * 0.85);
                    ctx.stroke();
                } else if (root.iconName === "undo") {
                    // Undo curved arrow
                    ctx.beginPath();
                    ctx.moveTo(w * 0.3, h * 0.3);
                    ctx.lineTo(w * 0.1, h * 0.5);
                    ctx.lineTo(w * 0.3, h * 0.7);
                    ctx.moveTo(w * 0.15, h * 0.5);
                    ctx.arc(w * 0.55, h * 0.5, w * 0.35, Math.PI, 0, false);
                    ctx.stroke();
                } else if (root.iconName === "redo") {
                    // Redo curved arrow
                    ctx.beginPath();
                    ctx.moveTo(w * 0.7, h * 0.3);
                    ctx.lineTo(w * 0.9, h * 0.5);
                    ctx.lineTo(w * 0.7, h * 0.7);
                    ctx.moveTo(w * 0.85, h * 0.5);
                    ctx.arc(w * 0.45, h * 0.5, w * 0.35, 0, Math.PI, false);
                    ctx.stroke();
                } else if (root.iconName === "trash") {
                    // Trash clear icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.2, h * 0.3);
                    ctx.lineTo(w * 0.8, h * 0.3);
                    ctx.moveTo(w * 0.4, h * 0.3);
                    ctx.lineTo(w * 0.4, h * 0.15);
                    ctx.lineTo(w * 0.6, h * 0.15);
                    ctx.lineTo(w * 0.6, h * 0.3);
                    ctx.moveTo(w * 0.25, h * 0.3);
                    ctx.lineTo(w * 0.3, h * 0.85);
                    ctx.lineTo(w * 0.7, h * 0.85);
                    ctx.lineTo(w * 0.75, h * 0.3);
                    ctx.stroke();
                } else if (root.iconName === "camera") {
                    // Screenshot camera icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.15, h * 0.35);
                    ctx.lineTo(w * 0.3, h * 0.35);
                    ctx.lineTo(w * 0.4, h * 0.2);
                    ctx.lineTo(w * 0.6, h * 0.2);
                    ctx.lineTo(w * 0.7, h * 0.35);
                    ctx.lineTo(w * 0.85, h * 0.35);
                    ctx.lineTo(w * 0.85, h * 0.8);
                    ctx.lineTo(w * 0.15, h * 0.8);
                    ctx.closePath();
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.55, 3.5, 0, 2 * Math.PI);
                    ctx.stroke();
                } else if (root.iconName === "close") {
                    // Close 'X' icon
                    ctx.beginPath();
                    ctx.moveTo(w * 0.25, h * 0.25);
                    ctx.lineTo(w * 0.75, h * 0.75);
                    ctx.moveTo(w * 0.75, h * 0.25);
                    ctx.lineTo(w * 0.25, h * 0.75);
                    ctx.stroke();
                } else if (root.iconName === "minimize") {
                    // Collapse chevron
                    ctx.beginPath();
                    ctx.moveTo(w * 0.3, h * 0.4);
                    ctx.lineTo(w * 0.5, h * 0.6);
                    ctx.lineTo(w * 0.7, h * 0.4);
                    ctx.stroke();
                }
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: !root.isDisabled
        cursorShape: root.isDisabled ? Qt.ArrowCursor : Qt.PointingHandCursor
        onClicked: {
            if (!root.isDisabled) {
                root.clicked()
            }
        }
    }

    // Modern Tooltip
    ToolTip {
        id: tooltip
        visible: mouseArea.containsMouse && root.tooltipText !== ""
        text: root.tooltipText
        delay: 500
        timeout: 3000

        contentItem: Text {
            text: tooltip.text
            font.pixelSize: 11
            font.bold: true
            color: "#f8fafc"
        }

        background: Rectangle {
            color: "#0f172a"
            radius: 6
            border.color: Qt.rgba(255, 255, 255, 0.15)
            border.width: 1
        }
    }
}
