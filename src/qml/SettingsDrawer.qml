import QtQuick
import QtQuick.Controls

Item {
    id: root

    property bool isOpened: false
    property int currentTab: 0 // 0: Renk & Fırça, 1: Lazer & Efekt, 2: Kısayollar & Genel

    property real currentHue: 0.0
    property real currentSat: 1.0
    property real currentVal: 1.0

    width: 330
    height: isOpened ? 410 : 0
    visible: isOpened
    clip: true

    Behavior on height { NumberAnimation { duration: 200; easing.type: Easing.OutQuad } }

    function updateColorFromHSV() {
        var c = Qt.hsva(currentHue / 360.0, currentSat, currentVal, 1.0);
        toolState.selectColor(c);
    }

    Rectangle {
        id: panelCard
        anchors.fill: parent
        radius: 18
        color: Qt.rgba(15/255, 23/255, 42/255, 0.98)
        border.color: Qt.rgba(255, 255, 255, 0.18)
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            // 1. Header Bar
            Row {
                width: parent.width
                spacing: 8

                Image {
                    source: "qrc:/resources/icons/drawonscreen.svg"
                    width: 20
                    height: 20
                    sourceSize.width: 40
                    sourceSize.height: 40
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: "DrawOnScreen Ayarları"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#f8fafc"
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { width: 105 }
                IconButton {
                    iconName: "close"
                    width: 24
                    height: 24
                    tooltipText: "Kapat"
                    onClicked: root.isOpened = false
                }
            }

            // 2. Tab Selector
            Rectangle {
                width: parent.width
                height: 30
                radius: 8
                color: Qt.rgba(255, 255, 255, 0.06)

                Row {
                    anchors.centerIn: parent
                    spacing: 4

                    // Tab 0: Renk & Fırça
                    Rectangle {
                        width: 96
                        height: 24
                        radius: 6
                        color: root.currentTab === 0 ? Qt.rgba(56/255, 189/255, 248/255, 0.25) : "transparent"
                        border.color: root.currentTab === 0 ? "#38bdf8" : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "🎨 Renk & Fırça"
                            font.pixelSize: 10
                            font.bold: root.currentTab === 0
                            color: root.currentTab === 0 ? "#38bdf8" : "#94a3b8"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentTab = 0
                        }
                    }

                    // Tab 1: Efekt & Yazı
                    Rectangle {
                        width: 100
                        height: 24
                        radius: 6
                        color: root.currentTab === 1 ? Qt.rgba(56/255, 189/255, 248/255, 0.25) : "transparent"
                        border.color: root.currentTab === 1 ? "#38bdf8" : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "👻 Efekt & Yazı"
                            font.pixelSize: 10
                            font.bold: root.currentTab === 1
                            color: root.currentTab === 1 ? "#38bdf8" : "#94a3b8"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentTab = 1
                        }
                    }

                    // Tab 2: Kısayollar
                    Rectangle {
                        width: 96
                        height: 24
                        radius: 6
                        color: root.currentTab === 2 ? Qt.rgba(56/255, 189/255, 248/255, 0.25) : "transparent"
                        border.color: root.currentTab === 2 ? "#38bdf8" : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "⌨️ Kısayollar"
                            font.pixelSize: 10
                            font.bold: root.currentTab === 2
                            color: root.currentTab === 2 ? "#38bdf8" : "#94a3b8"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentTab = 2
                        }
                    }
                }
            }

            // ==================== TAB 0: RENK & FIRÇA ====================
            Column {
                visible: root.currentTab === 0
                width: parent.width
                spacing: 10

                Row {
                    spacing: 12
                    anchors.horizontalCenter: parent.horizontalCenter

                    // Chromatic Wheel Canvas
                    Item {
                        width: 120
                        height: 120
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

                        // Selector Dot
                        Rectangle {
                            width: 10
                            height: 10
                            radius: 5
                            border.color: "#ffffff"
                            border.width: 1.5
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

                    // Brightness & Info
                    Column {
                        spacing: 6
                        anchors.verticalCenter: parent.verticalCenter

                        Row {
                            spacing: 6
                            Rectangle {
                                width: 20
                                height: 20
                                radius: 10
                                color: toolState.currentColor
                                border.color: "#ffffff"
                                border.width: 1.5
                            }
                            Text {
                                text: toolState.currentColor.toString().toUpperCase()
                                font.pixelSize: 11
                                font.bold: true
                                color: "#f8fafc"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        Text { text: "Parlaklık:"; font.pixelSize: 10; color: "#94a3b8" }
                        Slider {
                            from: 0.1
                            to: 1.0
                            value: root.currentVal
                            width: 120
                            onMoved: { root.currentVal = value; root.updateColorFromHSV() }
                        }

                        Text { text: "Fırça Kalınlığı (" + Math.round(toolState.currentWidth) + "px):"; font.pixelSize: 10; color: "#94a3b8" }
                        Slider {
                            from: 1
                            to: 40
                            value: toolState.currentWidth
                            width: 120
                            onMoved: toolState.selectWidth(value)
                        }
                    }
                }

                // Fill Shapes Toggle
                Row {
                    width: parent.width
                    spacing: 8
                    Text {
                        text: "Şekillerin İçini Doldur (Fill):"
                        font.pixelSize: 11
                        color: "#cbd5e1"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Item { width: 50 }
                    Switch {
                        checked: toolState.isFilled
                        onToggled: toolState.isFilled = checked
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // ==================== TAB 1: EFEKTLER & YAZI ====================
            Column {
                visible: root.currentTab === 1
                width: parent.width
                spacing: 12

                // Ghost Pen Lifetime
                Column {
                    width: parent.width
                    spacing: 2
                    Row {
                        width: parent.width
                        Text { text: "👻 Hayalet Kalem Silinme Süresi:"; font.pixelSize: 11; font.bold: true; color: "#f8fafc" }
                        Item { width: 20 }
                        Text {
                            text: (toolState.ghostDurationMs / 1000.0).toFixed(1) + " sn"
                            font.pixelSize: 11
                            font.bold: true
                            color: "#c084fc"
                        }
                    }
                    Slider {
                        width: parent.width
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: toolState.ghostDurationMs
                        onMoved: toolState.setGhostDuration(value)
                    }
                }

                // Text Font Size
                Column {
                    width: parent.width
                    spacing: 2
                    Row {
                        width: parent.width
                        Text { text: "🔤 Metin Yazı Boyutu:"; font.pixelSize: 11; font.bold: true; color: "#f8fafc" }
                        Item { width: 90 }
                        Text {
                            text: toolState.fontSize + " pt"
                            font.pixelSize: 11
                            font.bold: true
                            color: "#38bdf8"
                        }
                    }
                    Slider {
                        width: parent.width
                        from: 12
                        to: 64
                        stepSize: 2
                        value: toolState.fontSize
                        onMoved: toolState.fontSize = value
                    }
                }
            }

            // ==================== TAB 2: KISAYOLLARI ÖZELLEŞTİR ====================
            Column {
                visible: root.currentTab === 2
                width: parent.width
                spacing: 6

                Text {
                    text: "Özelleştirilebilir Kısayol Tuşları:"
                    font.pixelSize: 11
                    font.bold: true
                    color: "#f8fafc"
                }

                Column {
                    spacing: 5
                    width: parent.width

                    Row {
                        spacing: 8
                        width: parent.width
                        Text { text: "Çizim/Fare Modu:"; font.pixelSize: 10; color: "#cbd5e1"; width: 110; anchors.verticalCenter: parent.verticalCenter }
                        TextField {
                            id: modeInput
                            text: toolState.shortcutToggleMode
                            font.pixelSize: 10
                            width: 140
                            height: 24
                            color: "#f8fafc"
                            background: Rectangle { color: Qt.rgba(255, 255, 255, 0.08); radius: 4; border.color: modeInput.activeFocus ? "#38bdf8" : Qt.rgba(255, 255, 255, 0.2) }
                            onEditingFinished: toolState.shortcutToggleMode = text
                        }
                    }

                    Row {
                        spacing: 8
                        width: parent.width
                        Text { text: "Geri Al (Undo):"; font.pixelSize: 10; color: "#cbd5e1"; width: 110; anchors.verticalCenter: parent.verticalCenter }
                        TextField {
                            id: undoInput
                            text: toolState.shortcutUndo
                            font.pixelSize: 10
                            width: 140
                            height: 24
                            color: "#f8fafc"
                            background: Rectangle { color: Qt.rgba(255, 255, 255, 0.08); radius: 4; border.color: undoInput.activeFocus ? "#38bdf8" : Qt.rgba(255, 255, 255, 0.2) }
                            onEditingFinished: toolState.shortcutUndo = text
                        }
                    }

                    Row {
                        spacing: 8
                        width: parent.width
                        Text { text: "İleri Al (Redo):"; font.pixelSize: 10; color: "#cbd5e1"; width: 110; anchors.verticalCenter: parent.verticalCenter }
                        TextField {
                            id: redoInput
                            text: toolState.shortcutRedo
                            font.pixelSize: 10
                            width: 140
                            height: 24
                            color: "#f8fafc"
                            background: Rectangle { color: Qt.rgba(255, 255, 255, 0.08); radius: 4; border.color: redoInput.activeFocus ? "#38bdf8" : Qt.rgba(255, 255, 255, 0.2) }
                            onEditingFinished: toolState.shortcutRedo = text
                        }
                    }

                    Row {
                        spacing: 8
                        width: parent.width
                        Text { text: "Tümünü Temizle:"; font.pixelSize: 10; color: "#cbd5e1"; width: 110; anchors.verticalCenter: parent.verticalCenter }
                        TextField {
                            id: clearInput
                            text: toolState.shortcutClear
                            font.pixelSize: 10
                            width: 140
                            height: 24
                            color: "#f8fafc"
                            background: Rectangle { color: Qt.rgba(255, 255, 255, 0.08); radius: 4; border.color: clearInput.activeFocus ? "#38bdf8" : Qt.rgba(255, 255, 255, 0.2) }
                            onEditingFinished: toolState.shortcutClear = text
                        }
                    }

                    Row {
                        spacing: 8
                        width: parent.width
                        Text { text: "Ekran Görüntüsü:"; font.pixelSize: 10; color: "#cbd5e1"; width: 110; anchors.verticalCenter: parent.verticalCenter }
                        TextField {
                            id: ssInput
                            text: toolState.shortcutScreenshot
                            font.pixelSize: 10
                            width: 140
                            height: 24
                            color: "#f8fafc"
                            background: Rectangle { color: Qt.rgba(255, 255, 255, 0.08); radius: 4; border.color: ssInput.activeFocus ? "#38bdf8" : Qt.rgba(255, 255, 255, 0.2) }
                            onEditingFinished: toolState.shortcutScreenshot = text
                        }
                    }

                    Row {
                        spacing: 8
                        width: parent.width
                        Text { text: "Uygulamadan Çık:"; font.pixelSize: 10; color: "#cbd5e1"; width: 110; anchors.verticalCenter: parent.verticalCenter }
                        TextField {
                            id: exitInput
                            text: toolState.shortcutExit
                            font.pixelSize: 10
                            width: 140
                            height: 24
                            color: "#f8fafc"
                            background: Rectangle { color: Qt.rgba(255, 255, 255, 0.08); radius: 4; border.color: exitInput.activeFocus ? "#38bdf8" : Qt.rgba(255, 255, 255, 0.2) }
                            onEditingFinished: toolState.shortcutExit = text
                        }
                    }
                }

                Rectangle { width: parent.width; height: 1; color: Qt.rgba(255, 255, 255, 0.12) }

                Row {
                    width: parent.width
                    spacing: 12

                    Button {
                        text: "Varsayılana Sıfırla"
                        font.pixelSize: 10
                        onClicked: {
                            toolState.resetShortcutsToDefaults();
                            modeInput.text = toolState.shortcutToggleMode;
                            undoInput.text = toolState.shortcutUndo;
                            redoInput.text = toolState.shortcutRedo;
                            clearInput.text = toolState.shortcutClear;
                            ssInput.text = toolState.shortcutScreenshot;
                            exitInput.text = toolState.shortcutExit;
                        }
                    }

                    Button {
                        text: toolState.isVerticalLayout ? "Dikey Mod" : "Yatay Mod"
                        font.pixelSize: 10
                        onClicked: toolState.toggleOrientation()
                    }
                }
            }
        }
    }
}
