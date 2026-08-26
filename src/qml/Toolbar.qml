import QtQuick
import QtQuick.Controls

Item {
    id: root

    signal moveRequested(real dx, real dy)
    signal dragFinished()

    property bool isCollapsed: false
    property bool isVertical: toolState.isVerticalLayout
    property bool isSettingsOpen: false

    width: {
        if (isCollapsed) return collapsedPill.width
        if (isVertical) return isSettingsOpen ? (52 + 8 + 330) : 52
        return isSettingsOpen ? Math.max(contentRow.width + 20, 330) : (contentRow.width + 20)
    }

    height: {
        if (isCollapsed) return collapsedPill.height
        if (isVertical) return Math.max(contentCol.height + 20, isSettingsOpen ? 380 : 0)
        return isSettingsOpen ? (52 + 8 + 380) : 52
    }

    // ==================== 1. COLLAPSED PILL ====================
    Rectangle {
        id: collapsedPill
        visible: root.isCollapsed
        width: 100
        height: 44
        radius: 22
        color: Qt.rgba(15/255, 23/255, 42/255, 0.94)
        border.color: Qt.rgba(255, 255, 255, 0.18)
        border.width: 1

        Row {
            anchors.centerIn: parent
            spacing: 4

            Item {
                width: 16
                height: 32
                anchors.verticalCenter: parent.verticalCenter
                Canvas {
                    anchors.fill: parent
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.fillStyle = "#94a3b8";
                        for (var r = 0; r < 3; r++) {
                            for (var c = 0; c < 2; c++) {
                                ctx.fillRect(2 + c * 6, 8 + r * 6, 2, 2);
                            }
                        }
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.SizeAllCursor
                    property real lastX: 0
                    property real lastY: 0
                    property bool dragging: false
                    onPressed: (mouse) => { lastX = mouse.x; lastY = mouse.y; dragging = true }
                    onPositionChanged: (mouse) => {
                        if (dragging && (mouse.buttons & Qt.LeftButton)) {
                            var dx = mouse.x - lastX;
                            var dy = mouse.y - lastY;
                            if (dx !== 0 || dy !== 0) root.moveRequested(dx, dy);
                        }
                    }
                    onReleased: { dragging = false; root.dragFinished() }
                }
            }

            IconButton {
                iconName: toolState.interactionMode === 1 ? "mouse" : "pen"
                tooltipText: toolState.interactionMode === 1 ? "Çizim Moduna Geç (F9)" : "Masaüstü Moduna Geç (F9)"
                isActive: toolState.interactionMode === 0
                activeColor: "#38bdf8"
                onClicked: toolState.toggleInteractionMode()
            }

            IconButton {
                iconName: "minimize"
                rotation: 180
                tooltipText: "Genişlet"
                onClicked: root.isCollapsed = false
            }
        }
    }

    // ==================== 2. HORIZONTAL LAYOUT CONTAINER ====================
    Column {
        id: horizContainer
        visible: !root.isCollapsed && !root.isVertical
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 8

        // Settings Drawer
        SettingsDrawer {
            id: drawerHoriz
            isOpened: root.isSettingsOpen
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Main Horizontal Card
        Rectangle {
            id: mainCardHoriz
            width: contentRow.width + 20
            height: 52
            radius: 16
            color: Qt.rgba(15/255, 23/255, 42/255, 0.92)
            border.color: Qt.rgba(255, 255, 255, 0.16)
            border.width: 1

            // Drag handle across the whole card
            MouseArea {
                anchors.fill: parent
                z: -1
                cursorShape: Qt.SizeAllCursor
                property real lastX: 0
                property real lastY: 0
                property bool dragging: false
                onPressed: (mouse) => { lastX = mouse.x; lastY = mouse.y; dragging = true }
                onPositionChanged: (mouse) => {
                    if (dragging && (mouse.buttons & Qt.LeftButton)) {
                        var dx = mouse.x - lastX;
                        var dy = mouse.y - lastY;
                        if (dx !== 0 || dy !== 0) root.moveRequested(dx, dy);
                    }
                }
                onReleased: dragging = false
            }

            Row {
                id: contentRow
                anchors.centerIn: parent
                spacing: 4

                // Handle
                Item {
                    width: 14
                    height: 36
                    anchors.verticalCenter: parent.verticalCenter
                    Canvas {
                        anchors.fill: parent
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.fillStyle = "#94a3b8";
                            for (var r = 0; r < 4; r++) {
                                for (var c = 0; c < 2; c++) {
                                    ctx.fillRect(2 + c * 5, 8 + r * 6, 2, 2);
                                }
                            }
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.SizeAllCursor
                        property real lastX: 0
                        property real lastY: 0
                        property bool dragging: false
                        onPressed: (mouse) => { lastX = mouse.x; lastY = mouse.y; dragging = true }
                        onPositionChanged: (mouse) => {
                            if (dragging && (mouse.buttons & Qt.LeftButton)) {
                                var dx = mouse.x - lastX;
                                var dy = mouse.y - lastY;
                                if (dx !== 0 || dy !== 0) root.moveRequested(dx, dy);
                            }
                        }
                        onReleased: { dragging = false; root.dragFinished() }
                    }
                }

                // Logo Brand Icon
                Image {
                    source: "qrc:/resources/icons/drawonscreen.svg"
                    width: 22
                    height: 22
                    sourceSize.width: 44
                    sourceSize.height: 44
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                }

                // Mouse
                IconButton {
                    iconName: "mouse"
                    tooltipText: "Masaüstü Modu (Tıklama / İşaretçi) [F9]"
                    isActive: toolState.interactionMode === 1
                    activeColor: "#22c55e"
                    onClicked: toolState.setInteractionModeEnum(1)
                }

                Rectangle { width: 1; height: 24; color: Qt.rgba(255, 255, 255, 0.12); anchors.verticalCenter: parent.verticalCenter }

                // Pen
                IconButton {
                    iconName: "pen"
                    tooltipText: "Kalem (Çizim)"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 0
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(0) }
                }

                // Ghost Pen
                IconButton {
                    iconName: "ghost"
                    tooltipText: "Hayalet Kalem (Sönen sunum kalemi)"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 3
                    activeColor: "#c084fc"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(3) }
                }

                // Highlighter
                IconButton {
                    iconName: "highlighter"
                    tooltipText: "Fosforlu Kalem (Highlighter)"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 1
                    activeColor: "#fbbf24"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(1) }
                }

                // Laser
                IconButton {
                    iconName: "laser"
                    tooltipText: "Lazer İşaretçi (144Hz Glow)"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 2
                    activeColor: "#f43f5e"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(2) }
                }

                // Eraser
                IconButton {
                    iconName: "eraser"
                    tooltipText: "Vektörel Silgi"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 4
                    activeColor: "#f87171"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(4) }
                }

                Rectangle { width: 1; height: 24; color: Qt.rgba(255, 255, 255, 0.12); anchors.verticalCenter: parent.verticalCenter }

                // Shapes
                IconButton {
                    iconName: "line"
                    tooltipText: "Düz Çizgi"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 5
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(5) }
                }
                IconButton {
                    iconName: "arrow"
                    tooltipText: "Ok Çizimi"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 6
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(6) }
                }
                IconButton {
                    iconName: "rect"
                    tooltipText: "Dikdörtgen"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 7
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(7) }
                }
                IconButton {
                    iconName: "ellipse"
                    tooltipText: "Daire / Elips"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 8
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(8) }
                }

                // Text
                IconButton {
                    iconName: "text"
                    tooltipText: "Metin Ekle (Ekranda canlı yaz)"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 9
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(9) }
                }

                Rectangle { width: 1; height: 24; color: Qt.rgba(255, 255, 255, 0.12); anchors.verticalCenter: parent.verticalCenter }

                // Color Palette
                ColorPalette {
                    id: paletteHoriz
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Size Slider
                SizeSlider {
                    id: sliderHoriz
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Settings & Color Wheel Button
                IconButton {
                    iconName: "gear"
                    tooltipText: "Gelişmiş Ayarlar & Renk Çemberi"
                    isActive: root.isSettingsOpen
                    activeColor: "#a855f7"
                    onClicked: root.isSettingsOpen = !root.isSettingsOpen
                }

                Rectangle { width: 1; height: 24; color: Qt.rgba(255, 255, 255, 0.12); anchors.verticalCenter: parent.verticalCenter }

                // Whiteboard
                IconButton {
                    iconName: "board"
                    tooltipText: toolState.backgroundMode === 0 ? "Beyaz Tahtaya Geç" : (toolState.backgroundMode === 1 ? "Kara Tahtaya Geç" : "Şeffaf Ekrana Geç")
                    isActive: toolState.backgroundMode !== 0
                    activeColor: toolState.backgroundMode === 1 ? "#ffffff" : "#64748b"
                    onClicked: toolState.cycleBackgroundMode()
                }

                // Visibility Eye
                IconButton {
                    iconName: toolState.areDrawingsVisible ? "eye" : "eye-off"
                    tooltipText: toolState.areDrawingsVisible ? "Çizimleri Geçici Olarak Gizle" : "Çizimleri Tekrar Göster"
                    isActive: !toolState.areDrawingsVisible
                    activeColor: "#f59e0b"
                    onClicked: toolState.toggleDrawingsVisibility()
                }

                // Orientation Toggle
                IconButton {
                    iconName: "rotate"
                    tooltipText: "Dikey Konuma Geç"
                    onClicked: toolState.toggleOrientation()
                }

                Rectangle { width: 1; height: 24; color: Qt.rgba(255, 255, 255, 0.12); anchors.verticalCenter: parent.verticalCenter }

                // Undo / Redo / Clear / Camera / Minimize / Close
                IconButton {
                    iconName: "undo"
                    tooltipText: "Geri Al (Ctrl+Z)"
                    isDisabled: !toolState.canUndo
                    onClicked: toolState.undoRequested()
                }
                IconButton {
                    iconName: "redo"
                    tooltipText: "İleri Al (Ctrl+Y)"
                    isDisabled: !toolState.canRedo
                    onClicked: toolState.redoRequested()
                }
                IconButton {
                    iconName: "trash"
                    tooltipText: "Tüm Çizimleri Temizle (C)"
                    onClicked: toolState.clearRequested()
                }
                IconButton {
                    iconName: "camera"
                    tooltipText: "Ekran Görüntüsü Al (Ctrl+Shift+S)"
                    onClicked: toolState.screenshotRequested()
                }
                IconButton {
                    iconName: "minimize"
                    tooltipText: "Küçült"
                    onClicked: root.isCollapsed = true
                }
                IconButton {
                    iconName: "close"
                    tooltipText: "Çıkış (Alt+F4)"
                    activeColor: "#ef4444"
                    onClicked: toolState.exitRequested()
                }
            }
        }
    }

    // ==================== 3. VERTICAL LAYOUT CONTAINER ====================
    Row {
        id: vertContainer
        visible: !root.isCollapsed && root.isVertical
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        // Main Vertical Card
        Rectangle {
            id: mainCardVert
            width: 52
            height: contentCol.height + 20
            radius: 16
            color: Qt.rgba(15/255, 23/255, 42/255, 0.92)
            border.color: Qt.rgba(255, 255, 255, 0.16)
            border.width: 1

            // Drag handle across the whole card
            MouseArea {
                anchors.fill: parent
                z: -1
                cursorShape: Qt.SizeAllCursor
                property real lastX: 0
                property real lastY: 0
                property bool dragging: false
                onPressed: (mouse) => { lastX = mouse.x; lastY = mouse.y; dragging = true }
                onPositionChanged: (mouse) => {
                    if (dragging && (mouse.buttons & Qt.LeftButton)) {
                        var dx = mouse.x - lastX;
                        var dy = mouse.y - lastY;
                        if (dx !== 0 || dy !== 0) root.moveRequested(dx, dy);
                    }
                }
                onReleased: dragging = false
            }

            Column {
                id: contentCol
                anchors.centerIn: parent
                spacing: 3

                // Drag handle top
                Item {
                    width: 36
                    height: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                    Canvas {
                        anchors.fill: parent
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.fillStyle = "#94a3b8";
                            for (var r = 0; r < 2; r++) {
                                for (var c = 0; c < 4; c++) {
                                    ctx.fillRect(8 + c * 6, 3 + r * 5, 2, 2);
                                }
                            }
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.SizeAllCursor
                        property real lastX: 0
                        property real lastY: 0
                        property bool dragging: false
                        onPressed: (mouse) => { lastX = mouse.x; lastY = mouse.y; dragging = true }
                        onPositionChanged: (mouse) => {
                            if (dragging && (mouse.buttons & Qt.LeftButton)) {
                                var dx = mouse.x - lastX;
                                var dy = mouse.y - lastY;
                                if (dx !== 0 || dy !== 0) root.moveRequested(dx, dy);
                            }
                        }
                        onReleased: { dragging = false; root.dragFinished() }
                    }
                }

                // Logo Brand Icon
                Image {
                    source: "qrc:/resources/icons/drawonscreen.svg"
                    width: 22
                    height: 22
                    sourceSize.width: 44
                    sourceSize.height: 44
                    anchors.horizontalCenter: parent.horizontalCenter
                    fillMode: Image.PreserveAspectFit
                }

                // Mouse
                IconButton {
                    iconName: "mouse"
                    tooltipText: "Masaüstü Modu (Tıklama / İşaretçi) [F9]"
                    isActive: toolState.interactionMode === 1
                    activeColor: "#22c55e"
                    onClicked: toolState.setInteractionModeEnum(1)
                }

                Rectangle { width: 28; height: 1; color: Qt.rgba(255, 255, 255, 0.12); anchors.horizontalCenter: parent.horizontalCenter }

                // Tools
                IconButton {
                    iconName: "pen"
                    tooltipText: "Kalem (Çizim)"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 0
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(0) }
                }
                IconButton {
                    iconName: "ghost"
                    tooltipText: "Hayalet Kalem (Sönen)"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 3
                    activeColor: "#c084fc"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(3) }
                }
                IconButton {
                    iconName: "highlighter"
                    tooltipText: "Fosforlu Kalem"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 1
                    activeColor: "#fbbf24"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(1) }
                }
                IconButton {
                    iconName: "laser"
                    tooltipText: "Lazer İşaretçi"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 2
                    activeColor: "#f43f5e"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(2) }
                }
                IconButton {
                    iconName: "eraser"
                    tooltipText: "Vektörel Silgi"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 4
                    activeColor: "#f87171"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(4) }
                }

                Rectangle { width: 28; height: 1; color: Qt.rgba(255, 255, 255, 0.12); anchors.horizontalCenter: parent.horizontalCenter }

                // Shapes
                IconButton {
                    iconName: "line"
                    tooltipText: "Düz Çizgi"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 5
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(5) }
                }
                IconButton {
                    iconName: "arrow"
                    tooltipText: "Ok Çizimi"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 6
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(6) }
                }
                IconButton {
                    iconName: "rect"
                    tooltipText: "Dikdörtgen"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 7
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(7) }
                }
                IconButton {
                    iconName: "ellipse"
                    tooltipText: "Daire"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 8
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(8) }
                }
                IconButton {
                    iconName: "text"
                    tooltipText: "Metin Ekle"
                    isActive: toolState.interactionMode === 0 && toolState.currentTool === 9
                    activeColor: "#38bdf8"
                    onClicked: { toolState.setInteractionModeEnum(0); toolState.selectTool(9) }
                }

                Rectangle { width: 28; height: 1; color: Qt.rgba(255, 255, 255, 0.12); anchors.horizontalCenter: parent.horizontalCenter }

                // Palette & Settings
                ColorPalette {
                    id: paletteVert
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                IconButton {
                    iconName: "gear"
                    tooltipText: "Gelişmiş Ayarlar"
                    isActive: root.isSettingsOpen
                    activeColor: "#a855f7"
                    onClicked: root.isSettingsOpen = !root.isSettingsOpen
                }

                Rectangle { width: 28; height: 1; color: Qt.rgba(255, 255, 255, 0.12); anchors.horizontalCenter: parent.horizontalCenter }

                // Extras
                IconButton {
                    iconName: "board"
                    tooltipText: "Tahta Modu (Beyaz/Kara/Şeffaf)"
                    isActive: toolState.backgroundMode !== 0
                    activeColor: toolState.backgroundMode === 1 ? "#ffffff" : "#64748b"
                    onClicked: toolState.cycleBackgroundMode()
                }
                IconButton {
                    iconName: toolState.areDrawingsVisible ? "eye" : "eye-off"
                    tooltipText: "Çizimleri Gizle/Göster"
                    isActive: !toolState.areDrawingsVisible
                    activeColor: "#f59e0b"
                    onClicked: toolState.toggleDrawingsVisibility()
                }
                IconButton {
                    iconName: "rotate"
                    tooltipText: "Yatay Konuma Geç"
                    onClicked: toolState.toggleOrientation()
                }

                Rectangle { width: 28; height: 1; color: Qt.rgba(255, 255, 255, 0.12); anchors.horizontalCenter: parent.horizontalCenter }

                // Actions
                IconButton {
                    iconName: "undo"
                    tooltipText: "Geri Al (Ctrl+Z)"
                    isDisabled: !toolState.canUndo
                    onClicked: toolState.undoRequested()
                }
                IconButton {
                    iconName: "redo"
                    tooltipText: "İleri Al (Ctrl+Y)"
                    isDisabled: !toolState.canRedo
                    onClicked: toolState.redoRequested()
                }
                IconButton {
                    iconName: "trash"
                    tooltipText: "Temizle (C)"
                    onClicked: toolState.clearRequested()
                }
                IconButton {
                    iconName: "camera"
                    tooltipText: "Ekran Görüntüsü (Ctrl+Shift+S)"
                    onClicked: toolState.screenshotRequested()
                }
                IconButton {
                    iconName: "minimize"
                    tooltipText: "Küçült"
                    onClicked: root.isCollapsed = true
                }
                IconButton {
                    iconName: "close"
                    tooltipText: "Çıkış (Alt+F4)"
                    activeColor: "#ef4444"
                    onClicked: toolState.exitRequested()
                }
            }
        }

        // Settings Drawer in Vertical Mode
        SettingsDrawer {
            id: drawerVert
            isOpened: root.isSettingsOpen
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
