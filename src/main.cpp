#include <QApplication>
#include <QIcon>
#include <QScreen>
#include <QDebug>
#include <cstdlib>

#include "core/document.h"
#include "core/toolstatemanager.h"
#include "core/laserengine.h"
#include "ui/overlaywindow.h"
#include "system/shortcutmanager.h"
#include "system/screenshotmanager.h"

int main(int argc, char** argv)
{
    // Prefer Wayland native platform on Wayland sessions
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        const char* waylandDisplay = std::getenv("WAYLAND_DISPLAY");
        if (waylandDisplay && waylandDisplay[0] != '\0') {
            qputenv("QT_QPA_PLATFORM", "wayland");
        }
    }

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("drawonscreen"));
    app.setApplicationDisplayName(QStringLiteral("DrawOnScreen"));
    app.setOrganizationName(QStringLiteral("DrawOnScreen"));
    app.setOrganizationDomain(QStringLiteral("drawonscreen.local"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setWindowIcon(QIcon(QStringLiteral(":/resources/icons/drawonscreen.svg")));

    // Core instances
    DrawOnScreen::CanvasDocument document;
    DrawOnScreen::ToolStateManager stateManager;
    DrawOnScreen::LaserPointerEngine laserEngine;

    // Unified Full-Screen Overlay (Instant 0ms dragging + hardware-accelerated drawing)
    DrawOnScreen::OverlayWindow overlayWindow(&stateManager, &document, &laserEngine);

    // System integrations
    DrawOnScreen::ShortcutManager shortcutManager(&stateManager);
    DrawOnScreen::ScreenshotManager screenshotManager(&document, &stateManager);

    // Dynamic screen geometry update on monitor config changes
    QObject::connect(&app, &QGuiApplication::primaryScreenChanged, &overlayWindow, [&overlayWindow](QScreen*) {
        overlayWindow.updateScreenGeometry();
    });

    overlayWindow.show();

    qInfo() << "DrawOnScreen başlatıldı. KWin Wayland 0ms Instant LayerShell Overlay hazır.";
    qInfo() << "Kısayollar: F9 (Masaüstü/Çizim Modu), Ctrl+Z (Geri Al), Ctrl+Y (İleri Al), Ctrl+Shift+C (Temizle), Ctrl+Shift+S (Ekran Görüntüsü)";

    return app.exec();
}
