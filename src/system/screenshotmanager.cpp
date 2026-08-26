#include "screenshotmanager.h"
#include "../core/document.h"
#include "../core/toolstatemanager.h"
#include <QGuiApplication>
#include <QScreen>
#include <QClipboard>
#include <QPainter>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>

namespace DrawOnScreen {

ScreenshotManager::ScreenshotManager(CanvasDocument* doc, ToolStateManager* stateMgr, QObject* parent)
    : QObject(parent)
    , m_document(doc)
    , m_stateMgr(stateMgr)
{
    connect(m_stateMgr, &ToolStateManager::requestScreenshot, this, &ScreenshotManager::captureAndSave);
}

void ScreenshotManager::captureAndSave()
{
    auto* screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    // 1. Grab screen background
    QPixmap bgPixmap = screen->grabWindow(0);
    if (bgPixmap.isNull()) {
        // Fallback: create white/transparent background with screen geometry
        bgPixmap = QPixmap(screen->size());
        bgPixmap.fill(Qt::white);
    }

    // 2. Composite drawing strokes on top
    QPixmap finalPixmap = bgPixmap;
    {
        QPainter p(&finalPixmap);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        m_document->render(p);
    }

    // 3. Copy to system clipboard
    if (auto* clipboard = QGuiApplication::clipboard()) {
        clipboard->setPixmap(finalPixmap);
        qInfo() << "Ekran görüntüsü panoya kopyalandı.";
    }

    // 4. Save to Pictures/Screenshots directory
    const QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    const QString screenshotsDir = picturesPath + QStringLiteral("/Screenshots");
    QDir().mkpath(screenshotsDir);

    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss"));
    const QString filePath = QStringLiteral("%1/DrawOnScreen_%2.png").arg(screenshotsDir, timestamp);

    if (finalPixmap.save(filePath, "PNG")) {
        qInfo() << "Ekran görüntüsü kaydedildi:" << filePath;
        emit screenshotCaptured(filePath);
    }
}

} // namespace DrawOnScreen
