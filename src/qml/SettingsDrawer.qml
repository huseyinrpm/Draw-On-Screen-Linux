import QtQuick
import QtQuick.Controls

Item {
    id: root

    property bool isOpened: false
    property int currentTab: 0 // 0: Renk & Fırça, 1: Ölçek & Efekt, 2: Kısayollar & Genel
    property real uiScale: toolState ? toolState.uiScale : 1.0

    property real currentHue: 0.0
    property real currentSat: 1.0
    property real currentVal: 1.0

    width: Math.round(340 * uiScale)
    height: isOpened ? Math.round(450 * uiScale) : 0
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
        radius: Math.round(18 * root.uiScale)
        color: Qt.rgba(15/255, 23/255, 42/255, 0.98)
        border.color: Qt.rgba(255, 255, 255, 0.18)
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: Math.round(12 * root.uiScale)
            spacing: Math.round(10 * root.uiScale)

            // 1. Header Bar
            Row {
                width: parent.width
                spacing: 8

                Image {
                    source: "qrc:/resources/icons/drawonscreen.svg"
                    width: Math.round(20 * root.uiScale)
                    height: Math.round(20 * root.uiScale)
                    sourceSize.width: 40
                    sourceSize.height: 40
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: "DrawOnScreen Ayarları"
                    font.pixelSize: Math.round(12 * root.uiScale)
                    font.bold: true
                    color: "#f8fafc"
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { width: 105 }
                IconButton {
                    iconName: "close"
                    width: Math.round(24 * root.uiScale)
                    height: Math.round(24 * root.uiScale)
                    tooltipText: "Kapat"
                    onClicked: root.isOpened = false
                }
            }

            // 2. Tab Selector
            Rectangle {
                width: parent.width
                height: Math.round(30 * root.uiScale)
                radius: Math.round(8 * root.uiScale)
                color: Qt.rgba(255, 255, 255, 0.06)

                Row {
                    anchors.centerIn: parent
                    spacing: 4

                    // Tab 0: Renk & Fırça
                    Rectangle {
                        width: Math.round(96 * root.uiScale)
                        height: Math.round(24 * root.uiScale)
                        radius: Math.round(6 * root.uiScale)
                        color: root.currentTab === 0 ? Qt.rgba(56/255, 189/255, 248/255, 0.25) : "transparent"
                        border.color: root.currentTab === 0 ? "#38bdf8" : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "🎨 Renk & Fırça"
                            font.pixelSize: Math.round(10 * root.uiScale)
                            font.bold: root.currentTab === 0
                            color: root.currentTab === 0 ? "#38bdf8" : "#94a3b8"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentTab = 0
                        }
                    }

                    // Tab 1: Ölçek & Efekt
                    Rectangle {
                        width: Math.round(100 * root.uiScale)
                        height: Math.round(24 * root.uiScale)
                        radius: Math.round(6 * root.uiScale)
                        color: root.currentTab === 1 ? Qt.rgba(56/255, 189/255, 248/255, 0.25) : "transparent"
                        border.color: root.currentTab === 1 ? "#38bdf8" : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "🔍 Ölçek & Efekt"
                            font.pixelSize: Math.round(10 * root.uiScale)
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
                        width: Math.round(96 * root.uiScale)
                        height: Math.round(24 * root.uiScale)
                        radius: Math.round(6 * root.uiScale)
                        color: root.currentTab === 2 ? Qt.rgba(56/255, 189/255, 248/255, 0.25) : "transparent"
                        border.color: root.currentTab === 2 ? "#38bdf8" : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "⌨️ Kısayollar"
                            font.pixelSize: Math.round(10 * root.uiScale)
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

            // 3. Tab Contents
            // ==================== TAB 0: RENK & FIRÇA ====================
            Column {
                visible: root.currentTab === 0
                width: parent.width
                spacing: 8

                // HSV Color Picker Wheel
                Rectangle {
                    width: 180
                    height: 180
                    radius: 90
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "transparent"

                    Canvas {
                        id: hsvCanvas
                        anchors.fill: parent
                        renderTarget: Canvas.FramebufferObject

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            var cx = width / 2;
                            var cy = height / 2;
                            var radius = width / 2;

                            for (var angle = 0; angle < 360; angle += 1) {
                                var rad = angle * Math.PI / 180;
                                ctx.beginPath();
                                ctx.moveTo(cx, cy);
                                ctx.arc(cx, cy, radius, rad, rad + (Math.PI / 180) * 1.5);
                                ctx.closePath();

                                var col = Qt.hsva(angle / 360.0, 1.0, 1.0, 1.0);
                                var grad = ctx.createRadialGradient(cx, cy, 0, cx, cy, radius);
                                grad.addColorStop(0, "#ffffff");
                                grad.addColorStop(1, col);
                                ctx.fillStyle = grad;
                                ctx.fill();
                            }
                        }
                    }

                    // Touch Indicator
                    Rectangle {
                        id: hsvThumb
                        width: 16
                        height: 16
                        radius: 8
                        color: toolState.currentColor
                        border.color: "#ffffff"
                        border.width: 2
                        x: 90 - 8
                        y: 90 - 8

                        Behavior on x { NumberAnimation { duration: 60 } }
                        Behavior on y { NumberAnimation { duration: 60 } }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.CrossCursor

                        function updateFromPos(mouse) {
                            var cx = width / 2;
                            var cy = height / 2;
                            var dx = mouse.x - cx;
                            var dy = mouse.y - cy;
                            var dist = Math.min(width / 2, Math.sqrt(dx * dx + dy * dy));
                            var angle = Math.atan2(dy, dx) * 180 / Math.PI;
                            if (angle < 0) angle += 360;

                            root.currentHue = angle;
                            root.currentSat = dist / (width / 2);
                            root.updateColorFromHSV();

                            hsvThumb.x = cx + Math.cos(angle * Math.PI / 180) * dist - 8;
                            hsvThumb.y = cy + Math.sin(angle * Math.PI / 180) * dist - 8;
                        }

                        onPressed: (mouse) => updateFromPos(mouse)
                        onPositionChanged: (mouse) => {
                            if (mouse.buttons & Qt.LeftButton) updateFromPos(mouse);
                        }
                    }
                }

                // Brightness / Value Slider
                Column {
                    width: parent.width
                    spacing: 2
                    Text { text: "💡 Renk Parlaklığı:"; font.pixelSize: 11; font.bold: true; color: "#f8fafc" }
                    Slider {
                        id: valSlider
                        width: parent.width
                        from: 0.1
                        to: 1.0
                        stepSize: 0.05
                        value: 1.0
                        onMoved: {
                            root.currentVal = value;
                            root.updateColorFromHSV();
                        }
                    }
                }

                // Fill toggle
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

            // ==================== TAB 1: ÖLÇEK & EFEKT ====================
            Column {
                visible: root.currentTab === 1
                width: parent.width
                spacing: 12

                // UI Scaling Control (Menü ve Buton Büyüklüğü)
                Column {
                    width: parent.width
                    spacing: 4

                    Row {
                        width: parent.width
                        Text { text: "🔍 Menü & Buton Ölçeği (UI Scale):"; font.pixelSize: 11; font.bold: true; color: "#f8fafc" }
                        Item { width: 10 }
                        Text {
                            text: "%" + Math.round(toolState.uiScale * 100)
                            font.pixelSize: 11
                            font.bold: true
                            color: "#38bdf8"
                        }
                    }

                    Slider {
                        id: uiScaleSlider
                        width: parent.width
                        from: 0.75
                        to: 1.60
                        stepSize: 0.05
                        value: toolState.uiScale
                        onMoved: toolState.setScale(value)
                    }

                    // Presets
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 6

                        Repeater {
                            model: [
                                { label: "%80", scale: 0.80 },
                                { label: "%100", scale: 1.0 },
                                { label: "%125", scale: 1.25 },
                                { label: "%150 (Tahta)", scale: 1.50 }
                            ]
                            delegate: Rectangle {
                                width: Math.round(62 * (root.uiScale > 1.2 ? 1.1 : 1.0))
                                height: 22
                                radius: 5
                                color: Math.abs(toolState.uiScale - modelData.scale) < 0.03 ? Qt.rgba(56/255, 189/255, 248/255, 0.3) : Qt.rgba(255, 255, 255, 0.08)
                                border.color: Math.abs(toolState.uiScale - modelData.scale) < 0.03 ? "#38bdf8" : Qt.rgba(255, 255, 255, 0.15)
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.label
                                    font.pixelSize: 9
                                    font.bold: true
                                    color: Math.abs(toolState.uiScale - modelData.scale) < 0.03 ? "#38bdf8" : "#cbd5e1"
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: toolState.setScale(modelData.scale)
                                }
                            }
                        }
                    }
                }

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
