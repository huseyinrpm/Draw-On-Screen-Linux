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

    width: 36
    height: 36

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 10
        color: {
            if (root.isDisabled) return "transparent"
            if (root.isActive) return Qt.rgba(56/255, 189/255, 248/255, 0.24)
            if (mouseArea.containsPress) return Qt.rgba(255, 255, 255, 0.20)
            if (mouseArea.containsMouse) return Qt.rgba(255, 255, 255, 0.12)
            return "transparent"
        }

        border.color: {
            if (root.isActive) return root.activeColor
            if (mouseArea.containsMouse && !root.isDisabled) return Qt.rgba(255, 255, 255, 0.28)
            return "transparent"
        }
        border.width: root.isActive ? 1.5 : 1

        Behavior on color { ColorAnimation { duration: 140 } }
        Behavior on border.color { ColorAnimation { duration: 140 } }

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

            Behavior on width { NumberAnimation { duration: 160; easing.type: Easing.OutQuad } }
        }

        // Custom color swatch badge
        Rectangle {
            id: colorSwatch
            visible: root.isCustomColorBadge
            anchors.centerIn: parent
            width: 20
            height: 20
            radius: 10
            color: root.customColor
            border.color: "#ffffff"
            border.width: 1.5
        }

        // Vector icon canvas
        Canvas {
            id: iconCanvas
            visible: !root.isCustomColorBadge
            anchors.fill: parent
            anchors.margins: 8
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
                    ctx.beginPath();
                    ctx.moveTo(w * 0.22, h * 0.12);
                    ctx.lineTo(w * 0.22, h * 0.88);
                    ctx.lineTo(w * 0.44, h * 0.66);
                    ctx.lineTo(w * 0.66, h * 0.92);
                    ctx.lineTo(w * 0.80, h * 0.80);
                    ctx.lineTo(w * 0.58, h * 0.54);
                    ctx.lineTo(w * 0.86, h * 0.54);
                    ctx.closePath();
                    ctx.fill();
                } else if (root.iconName === "pen") {
                    ctx.beginPath();
                    ctx.moveTo(w * 0.7, h * 0.1);
                    ctx.lineTo(w * 0.9, h * 0.3);
                    ctx.lineTo(w * 0.35, h * 0.85);
                    ctx.lineTo(w * 0.1, h * 0.9);
                    ctx.lineTo(w * 0.15, h * 0.65);
                    ctx.closePath();
                    ctx.stroke();
                } else if (root.iconName === "ghost") {
                    // Ghost / Sparkle pen
                    ctx.beginPath();
                    ctx.moveTo(w * 0.7, h * 0.15);
                    ctx.lineTo(w * 0.85, h * 0.3);
                    ctx.lineTo(w * 0.35, h * 0.8);
                    ctx.lineTo(w * 0.15, h * 0.85);
                    ctx.lineTo(w * 0.2, h * 0.65);
                    ctx.closePath();
                    ctx.stroke();
                    // Small sparkles
                    ctx.beginPath();
                    ctx.arc(w * 0.8, h * 0.7, 1.5, 0, 2*Math.PI);
                    ctx.arc(w * 0.6, h * 0.9, 1.2, 0, 2*Math.PI);
                    ctx.fill();
                } else if (root.iconName === "highlighter") {
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
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.5, 3.5, 0, 2 * Math.PI);
                    ctx.fill();
                    ctx.beginPath();
                    ctx.moveTo(w * 0.5, h * 0.1); ctx.lineTo(w * 0.5, h * 0.25);
                    ctx.moveTo(w * 0.5, h * 0.75); ctx.lineTo(w * 0.5, h * 0.9);
                    ctx.moveTo(w * 0.1, h * 0.5); ctx.lineTo(w * 0.25, h * 0.5);
                    ctx.moveTo(w * 0.75, h * 0.5); ctx.lineTo(w * 0.9, h * 0.5);
                    ctx.stroke();
                } else if (root.iconName === "eraser") {
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
                    ctx.beginPath();
                    ctx.moveTo(w * 0.15, h * 0.85);
                    ctx.lineTo(w * 0.85, h * 0.15);
                    ctx.stroke();
                } else if (root.iconName === "arrow") {
                    ctx.beginPath();
                    ctx.moveTo(w * 0.15, h * 0.85);
                    ctx.lineTo(w * 0.85, h * 0.15);
                    ctx.moveTo(w * 0.5, h * 0.15);
                    ctx.lineTo(w * 0.85, h * 0.15);
                    ctx.lineTo(w * 0.85, h * 0.5);
                    ctx.stroke();
                } else if (root.iconName === "rect") {
                    ctx.beginPath();
                    ctx.rect(w * 0.15, h * 0.2, w * 0.7, h * 0.6);
                    ctx.stroke();
                } else if (root.iconName === "ellipse") {
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.5, w * 0.35, 0, 2 * Math.PI);
                    ctx.stroke();
                } else if (root.iconName === "text") {
                    ctx.beginPath();
                    ctx.moveTo(w * 0.2, h * 0.2);
                    ctx.lineTo(w * 0.8, h * 0.2);
                    ctx.moveTo(w * 0.5, h * 0.2);
                    ctx.lineTo(w * 0.5, h * 0.85);
                    ctx.stroke();
                } else if (root.iconName === "eye") {
                    // Eye icon (visibility)
                    ctx.beginPath();
                    ctx.moveTo(w * 0.1, h * 0.5);
                    ctx.bezierCurveTo(w * 0.3, h * 0.2, w * 0.7, h * 0.2, w * 0.9, h * 0.5);
                    ctx.bezierCurveTo(w * 0.7, h * 0.8, w * 0.3, h * 0.8, w * 0.1, h * 0.5);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.5, 3, 0, 2 * Math.PI);
                    ctx.fill();
                } else if (root.iconName === "eye-off") {
                    // Eye off (hidden)
                    ctx.beginPath();
                    ctx.moveTo(w * 0.1, h * 0.5);
                    ctx.bezierCurveTo(w * 0.3, h * 0.2, w * 0.7, h * 0.2, w * 0.9, h * 0.5);
                    ctx.bezierCurveTo(w * 0.7, h * 0.8, w * 0.3, h * 0.8, w * 0.1, h * 0.5);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.moveTo(w * 0.15, h * 0.15);
                    ctx.lineTo(w * 0.85, h * 0.85);
                    ctx.stroke();
                } else if (root.iconName === "wheel") {
                    // Chromatic color wheel icon
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.5, w * 0.38, 0, 2 * Math.PI);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.5, w * 0.18, 0, 2 * Math.PI);
                    ctx.stroke();
                } else if (root.iconName === "gear" || root.iconName === "settings") {
                    // Cog / Gear icon
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.5, w * 0.2, 0, 2 * Math.PI);
                    ctx.stroke();
                    for (var ga = 0; ga < 360; ga += 45) {
                        var grad = ga * Math.PI / 180;
                        var gx1 = w * 0.5 + w * 0.28 * Math.cos(grad);
                        var gy1 = h * 0.5 + h * 0.28 * Math.sin(grad);
                        var gx2 = w * 0.5 + w * 0.40 * Math.cos(grad);
                        var gy2 = h * 0.5 + h * 0.40 * Math.sin(grad);
                        ctx.beginPath();
                        ctx.moveTo(gx1, gy1);
                        ctx.lineTo(gx2, gy2);
                        ctx.stroke();
                    }
                } else if (root.iconName === "board") {
                    // Blackboard / Whiteboard easel
                    ctx.beginPath();
                    ctx.rect(w * 0.15, h * 0.15, w * 0.7, h * 0.55);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.moveTo(w * 0.3, h * 0.7); ctx.lineTo(w * 0.2, h * 0.9);
                    ctx.moveTo(w * 0.7, h * 0.7); ctx.lineTo(w * 0.8, h * 0.9);
                    ctx.stroke();
                } else if (root.iconName === "spotlight") {
                    // Flashlight / Spotlight
                    ctx.beginPath();
                    ctx.moveTo(w * 0.3, h * 0.35);
                    ctx.lineTo(w * 0.6, h * 0.2);
                    ctx.lineTo(w * 0.8, h * 0.65);
                    ctx.lineTo(w * 0.5, h * 0.8);
                    ctx.closePath();
                    ctx.stroke();
                    // Light beam
                    ctx.beginPath();
                    ctx.moveTo(w * 0.3, h * 0.35); ctx.lineTo(w * 0.1, h * 0.2);
                    ctx.moveTo(w * 0.5, h * 0.8); ctx.lineTo(w * 0.1, h * 0.9);
                    ctx.stroke();
                } else if (root.iconName === "rotate") {
                    // Orientation rotate icon
                    ctx.beginPath();
                    ctx.arc(w * 0.5, h * 0.5, w * 0.35, -Math.PI*0.7, Math.PI*0.8);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.moveTo(w * 0.85, h * 0.3); ctx.lineTo(w * 0.85, h * 0.6); ctx.lineTo(w * 0.55, h * 0.6);
                    ctx.stroke();
                } else if (root.iconName === "undo") {
                    ctx.beginPath();
                    ctx.moveTo(w * 0.3, h * 0.3);
                    ctx.lineTo(w * 0.1, h * 0.5);
                    ctx.lineTo(w * 0.3, h * 0.7);
                    ctx.moveTo(w * 0.15, h * 0.5);
                    ctx.arc(w * 0.55, h * 0.5, w * 0.35, Math.PI, 0, false);
                    ctx.stroke();
                } else if (root.iconName === "redo") {
                    ctx.beginPath();
                    ctx.moveTo(w * 0.7, h * 0.3);
                    ctx.lineTo(w * 0.9, h * 0.5);
                    ctx.lineTo(w * 0.7, h * 0.7);
                    ctx.moveTo(w * 0.85, h * 0.5);
                    ctx.arc(w * 0.45, h * 0.5, w * 0.35, 0, Math.PI, false);
                    ctx.stroke();
                } else if (root.iconName === "trash") {
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
                    ctx.beginPath();
                    ctx.moveTo(w * 0.25, h * 0.25);
                    ctx.lineTo(w * 0.75, h * 0.75);
                    ctx.moveTo(w * 0.75, h * 0.25);
                    ctx.lineTo(w * 0.25, h * 0.75);
                    ctx.stroke();
                } else if (root.iconName === "minimize") {
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

    ToolTip {
        id: tooltip
        visible: mouseArea.containsMouse && root.tooltipText !== ""
        text: root.tooltipText
        delay: 400
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
            border.color: Qt.rgba(255, 255, 255, 0.18)
            border.width: 1
        }
    }
}
