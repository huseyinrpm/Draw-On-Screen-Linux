import QtQuick
import QtQuick.Controls

Item {
    id: root

    property bool isOpened: false
    property real currentHue: 0.0 // 0 - 360
    property real currentSat: 1.0 // 0 - 1
    property real currentVal: 1.0 // 0 - 1

    width: 220
    height: 290
    visible: isOpened

    function updateColorFromHSV() {
        var c = Qt.hsva(currentHue / 360.0, currentSat, currentVal, 1.0);
        toolState.selectColor(c);
    }

    Rectangle {
        id: card
        anchors.fill: parent
        radius: 16
        color: Qt.rgba(15/255, 23/255, 42/255, 0.96)
        border.color: Qt.rgba(255, 255, 255, 0.18)
        border.width: 1

        Column {
            anchors.centerIn: parent
            spacing: 8

            // Top Header: Title & Close
            Row {
                width: 196
                spacing: 8
                Text {
                    text: "Renk Çemberi & Ayarlar"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#f8fafc"
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { width: 30 }
                IconButton {
                    iconName: "close"
                    width: 24
                    height: 24
                    tooltipText: "Kapat"
                    onClicked: root.isOpened = false
                }
            }

            // Chromatic Color Wheel Canvas
            Item {
                width: 130
                height: 130
                anchors.horizontalCenter: parent.horizontalCenter

                Canvas {
                    id: wheelCanvas
                    anchors.fill: parent
                    renderTarget: Canvas.FramebufferObject

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();
                        var cx = width / 2;
                        var cy = height / 2;
                        var radius = width / 2 - 2;

                        for (var angle = 0; angle < 360; angle += 2) {
                            var startAngle = (angle - 1) * Math.PI / 180;
                            var endAngle = (angle + 2) * Math.PI / 180;
                            ctx.beginPath();
                            ctx.moveTo(cx, cy);
                            ctx.arc(cx, cy, radius, startAngle, endAngle);
                            ctx.closePath();

                            var grad = ctx.createRadialGradient(cx, cy, 0, cx, cy, radius);
                            grad.addColorStop(0, "rgba(255, 255, 255, 1)");
                            grad.addColorStop(1, "hsl(" + angle + ", 100%, 50%)");
                            ctx.fillStyle = grad;
                            ctx.fill();
                        }
                    }
                }

                // Selector Dot on Wheel
                Rectangle {
                    id: selectorDot
                    width: 12
                    height: 12
                    radius: 6
                    border.color: "#ffffff"
                    border.width: 2
                    color: toolState.currentColor

                    x: {
                        var cx = parent.width / 2;
                        var r = (parent.width / 2 - 4) * root.currentSat;
                        var rad = root.currentHue * Math.PI / 180;
                        return cx + r * Math.cos(rad) - width / 2;
                    }
                    y: {
                        var cy = parent.height / 2;
                        var r = (parent.height / 2 - 4) * root.currentSat;
                        var rad = root.currentHue * Math.PI / 180;
                        return cy + r * Math.sin(rad) - height / 2;
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.CrossCursor

                    function pickAt(mx, my) {
                        var cx = width / 2;
                        var cy = height / 2;
                        var dx = mx - cx;
                        var dy = my - cy;
                        var dist = Math.sqrt(dx * dx + dy * dy);
                        var maxR = width / 2 - 4;

                        var angle = Math.atan2(dy, dx) * 180 / Math.PI;
                        if (angle < 0) angle += 360;

                        root.currentHue = angle;
                        root.currentSat = Math.max(0.0, Math.min(1.0, dist / maxR));
                        root.updateColorFromHSV();
                    }

                    onPressed: (mouse) => pickAt(mouse.x, mouse.y)
                    onPositionChanged: (mouse) => pickAt(mouse.x, mouse.y)
                }
            }

            // Brightness / Value Slider
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 8
                Text {
                    text: "Parlaklık:"
                    font.pixelSize: 10
                    color: "#94a3b8"
                    anchors.verticalCenter: parent.verticalCenter
                }
                Slider {
                    id: valSlider
                    from: 0.1
                    to: 1.0
                    value: root.currentVal
                    width: 120
                    anchors.verticalCenter: parent.verticalCenter
                    onMoved: {
                        root.currentVal = value;
                        root.updateColorFromHSV();
                    }
                }
            }

            // Rainbow Mode Button
            Rectangle {
                width: 196
                height: 32
                radius: 8
                color: toolState.isRainbowMode ? Qt.rgba(56/255, 189/255, 248/255, 0.25) : Qt.rgba(255, 255, 255, 0.08)
                border.color: toolState.isRainbowMode ? "#38bdf8" : Qt.rgba(255, 255, 255, 0.15)
                border.width: toolState.isRainbowMode ? 1.5 : 1

                Row {
                    anchors.centerIn: parent
                    spacing: 6
                    // Rainbow Gradient Swatch
                    Rectangle {
                        width: 16
                        height: 16
                        radius: 8
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: "#ef4444" }
                            GradientStop { position: 0.33; color: "#eab308" }
                            GradientStop { position: 0.66; color: "#3b82f6" }
                            GradientStop { position: 1.0; color: "#a855f7" }
                        }
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: "🌈 Gökkuşağı (Rainbow) Çizim"
                        font.pixelSize: 11
                        font.bold: true
                        color: toolState.isRainbowMode ? "#38bdf8" : "#f8fafc"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        toolState.toggleRainbowMode()
                    }
                }
            }

            // Laser Fade Speed Slider
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 6
                Text {
                    text: "Lazer İzi:"
                    font.pixelSize: 10
                    color: "#94a3b8"
                    anchors.verticalCenter: parent.verticalCenter
                }
                Slider {
                    id: laserSlider
                    from: 100
                    to: 2500
                    stepSize: 100
                    value: toolState.laserFadeDuration
                    width: 100
                    anchors.verticalCenter: parent.verticalCenter
                    onMoved: {
                        toolState.setLaserFadeDurationMs(value)
                    }
                }
                Text {
                    text: (toolState.laserFadeDuration / 1000.0).toFixed(1) + "s"
                    font.pixelSize: 10
                    font.bold: true
                    color: "#f43f5e"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
