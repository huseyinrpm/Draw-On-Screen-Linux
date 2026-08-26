#pragma once

#include <QObject>
#include <QPixmap>
#include <QImage>

namespace DrawOnScreen {

class CanvasDocument;
class ToolStateManager;

class ScreenshotManager : public QObject {
    Q_OBJECT
public:
    explicit ScreenshotManager(CanvasDocument* doc, ToolStateManager* stateMgr, QObject* parent = nullptr);

public slots:
    void captureAndSave();

signals:
    void screenshotCaptured(const QString& filePath);

private:
    CanvasDocument* m_document = nullptr;
    ToolStateManager* m_stateMgr = nullptr;
};

} // namespace DrawOnScreen
