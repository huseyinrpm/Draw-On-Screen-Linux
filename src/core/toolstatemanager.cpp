#include "toolstatemanager.h"
#include <QSettings>

namespace DrawOnScreen {

ToolStateManager::ToolStateManager(QObject* parent)
    : QObject(parent)
{
    loadSettings();
}

ToolStateManager::~ToolStateManager()
{
    saveSettings();
}

void ToolStateManager::loadSettings()
{
    QSettings settings(QStringLiteral("DrawOnScreen"), QStringLiteral("Settings"));
    m_currentTool = static_cast<ToolType>(settings.value(QStringLiteral("currentTool"), static_cast<int>(ToolType::Pen)).toInt());
    m_currentColor = QColor(settings.value(QStringLiteral("currentColor"), QStringLiteral("#ef4444")).toString());
    m_currentWidth = settings.value(QStringLiteral("currentWidth"), 4.0).toReal();
    m_isFilled = settings.value(QStringLiteral("isFilled"), false).toBool();
    m_isVerticalLayout = settings.value(QStringLiteral("isVerticalLayout"), false).toBool();
    m_ghostDurationMs = settings.value(QStringLiteral("ghostDurationMs"), 3000).toInt();
    m_fontSize = settings.value(QStringLiteral("fontSize"), 28).toInt();
    m_savedToolbarX = settings.value(QStringLiteral("toolbarX"), 80).toInt();
    m_savedToolbarY = settings.value(QStringLiteral("toolbarY"), 80).toInt();

    m_shortcutToggleMode = settings.value(QStringLiteral("shortcutToggleMode"), QStringLiteral("F9")).toString();
    m_shortcutUndo = settings.value(QStringLiteral("shortcutUndo"), QStringLiteral("Ctrl+Z")).toString();
    m_shortcutRedo = settings.value(QStringLiteral("shortcutRedo"), QStringLiteral("Ctrl+Y")).toString();
    m_shortcutClear = settings.value(QStringLiteral("shortcutClear"), QStringLiteral("Ctrl+Shift+C")).toString();
    m_shortcutScreenshot = settings.value(QStringLiteral("shortcutScreenshot"), QStringLiteral("Ctrl+Shift+S")).toString();
    m_shortcutExit = settings.value(QStringLiteral("shortcutExit"), QStringLiteral("Alt+F4")).toString();
}

void ToolStateManager::saveSettings()
{
    QSettings settings(QStringLiteral("DrawOnScreen"), QStringLiteral("Settings"));
    settings.setValue(QStringLiteral("currentTool"), static_cast<int>(m_currentTool));
    settings.setValue(QStringLiteral("currentColor"), m_currentColor.name(QColor::HexRgb));
    settings.setValue(QStringLiteral("currentWidth"), m_currentWidth);
    settings.setValue(QStringLiteral("isFilled"), m_isFilled);
    settings.setValue(QStringLiteral("isVerticalLayout"), m_isVerticalLayout);
    settings.setValue(QStringLiteral("ghostDurationMs"), m_ghostDurationMs);
    settings.setValue(QStringLiteral("fontSize"), m_fontSize);
    settings.setValue(QStringLiteral("toolbarX"), m_savedToolbarX);
    settings.setValue(QStringLiteral("toolbarY"), m_savedToolbarY);

    settings.setValue(QStringLiteral("shortcutToggleMode"), m_shortcutToggleMode);
    settings.setValue(QStringLiteral("shortcutUndo"), m_shortcutUndo);
    settings.setValue(QStringLiteral("shortcutRedo"), m_shortcutRedo);
    settings.setValue(QStringLiteral("shortcutClear"), m_shortcutClear);
    settings.setValue(QStringLiteral("shortcutScreenshot"), m_shortcutScreenshot);
    settings.setValue(QStringLiteral("shortcutExit"), m_shortcutExit);
}

void ToolStateManager::setShortcutToggleMode(const QString& seq)
{
    if (m_shortcutToggleMode != seq) {
        m_shortcutToggleMode = seq;
        emit shortcutToggleModeChanged(seq);
        emit shortcutsConfigChanged();
        saveSettings();
    }
}

void ToolStateManager::setShortcutUndo(const QString& seq)
{
    if (m_shortcutUndo != seq) {
        m_shortcutUndo = seq;
        emit shortcutUndoChanged(seq);
        emit shortcutsConfigChanged();
        saveSettings();
    }
}

void ToolStateManager::setShortcutRedo(const QString& seq)
{
    if (m_shortcutRedo != seq) {
        m_shortcutRedo = seq;
        emit shortcutRedoChanged(seq);
        emit shortcutsConfigChanged();
        saveSettings();
    }
}

void ToolStateManager::setShortcutClear(const QString& seq)
{
    if (m_shortcutClear != seq) {
        m_shortcutClear = seq;
        emit shortcutClearChanged(seq);
        emit shortcutsConfigChanged();
        saveSettings();
    }
}

void ToolStateManager::setShortcutScreenshot(const QString& seq)
{
    if (m_shortcutScreenshot != seq) {
        m_shortcutScreenshot = seq;
        emit shortcutScreenshotChanged(seq);
        emit shortcutsConfigChanged();
        saveSettings();
    }
}

void ToolStateManager::setShortcutExit(const QString& seq)
{
    if (m_shortcutExit != seq) {
        m_shortcutExit = seq;
        emit shortcutExitChanged(seq);
        emit shortcutsConfigChanged();
        saveSettings();
    }
}

void ToolStateManager::setCustomShortcut(const QString& action, const QString& keySeq)
{
    if (action == QStringLiteral("toggleMode")) setShortcutToggleMode(keySeq);
    else if (action == QStringLiteral("undo")) setShortcutUndo(keySeq);
    else if (action == QStringLiteral("redo")) setShortcutRedo(keySeq);
    else if (action == QStringLiteral("clear")) setShortcutClear(keySeq);
    else if (action == QStringLiteral("screenshot")) setShortcutScreenshot(keySeq);
    else if (action == QStringLiteral("exit")) setShortcutExit(keySeq);
}

void ToolStateManager::resetShortcutsToDefaults()
{
    setShortcutToggleMode(QStringLiteral("F9"));
    setShortcutUndo(QStringLiteral("Ctrl+Z"));
    setShortcutRedo(QStringLiteral("Ctrl+Y"));
    setShortcutClear(QStringLiteral("Ctrl+Shift+C"));
    setShortcutScreenshot(QStringLiteral("Ctrl+Shift+S"));
    setShortcutExit(QStringLiteral("Alt+F4"));
}

void ToolStateManager::saveToolbarPosition(int x, int y)
{
    m_savedToolbarX = x;
    m_savedToolbarY = y;
    emit savedToolbarXChanged(x);
    emit savedToolbarYChanged(y);
    saveSettings();
}

void ToolStateManager::setSavedToolbarX(int x)
{
    if (m_savedToolbarX != x) {
        m_savedToolbarX = x;
        emit savedToolbarXChanged(x);
    }
}

void ToolStateManager::setSavedToolbarY(int y)
{
    if (m_savedToolbarY != y) {
        m_savedToolbarY = y;
        emit savedToolbarYChanged(y);
    }
}

void ToolStateManager::setCurrentTool(ToolType tool)
{
    if (m_currentTool != tool) {
        m_currentTool = tool;
        emit currentToolChanged(static_cast<int>(tool));
        saveSettings();
    }
}

void ToolStateManager::setCurrentToolInt(int tool)
{
    setCurrentTool(static_cast<ToolType>(tool));
}

void ToolStateManager::setInteractionMode(InteractionMode mode)
{
    if (m_interactionMode != mode) {
        m_interactionMode = mode;
        emit interactionModeChanged(static_cast<int>(mode));
    }
}

void ToolStateManager::setInteractionModeInt(int mode)
{
    setInteractionMode(static_cast<InteractionMode>(mode));
}

void ToolStateManager::setBackgroundMode(BackgroundMode mode)
{
    if (m_backgroundMode != mode) {
        m_backgroundMode = mode;
        emit backgroundModeChanged(static_cast<int>(mode));
    }
}

void ToolStateManager::setBackgroundModeInt(int mode)
{
    setBackgroundMode(static_cast<BackgroundMode>(mode));
}

void ToolStateManager::setCurrentColor(const QColor& color)
{
    if (m_currentColor != color) {
        m_currentColor = color;
        emit currentColorChanged(color);
        saveSettings();
    }
}

void ToolStateManager::selectColor(const QColor& color)
{
    setCurrentColor(color);
}

void ToolStateManager::setCurrentWidth(qreal width)
{
    if (!qFuzzyCompare(m_currentWidth, width)) {
        m_currentWidth = width;
        emit currentWidthChanged(width);
        saveSettings();
    }
}

void ToolStateManager::setIsFilled(bool filled)
{
    if (m_isFilled != filled) {
        m_isFilled = filled;
        emit isFilledChanged(filled);
        saveSettings();
    }
}

void ToolStateManager::setCanUndo(bool can)
{
    if (m_canUndo != can) {
        m_canUndo = can;
        emit canUndoChanged(can);
    }
}

void ToolStateManager::setCanRedo(bool can)
{
    if (m_canRedo != can) {
        m_canRedo = can;
        emit canRedoChanged(can);
    }
}

void ToolStateManager::setIsToolbarVisible(bool visible)
{
    if (m_isToolbarVisible != visible) {
        m_isToolbarVisible = visible;
        emit isToolbarVisibleChanged(visible);
    }
}

void ToolStateManager::setIsVerticalLayout(bool vertical)
{
    if (m_isVerticalLayout != vertical) {
        m_isVerticalLayout = vertical;
        emit isVerticalLayoutChanged(vertical);
        saveSettings();
    }
}

void ToolStateManager::setAreDrawingsVisible(bool visible)
{
    if (m_areDrawingsVisible != visible) {
        m_areDrawingsVisible = visible;
        emit areDrawingsVisibleChanged(visible);
    }
}

void ToolStateManager::setGhostDurationMs(int ms)
{
    const int effective = qBound(500, ms, 15000);
    if (m_ghostDurationMs != effective) {
        m_ghostDurationMs = effective;
        emit ghostDurationMsChanged(effective);
        saveSettings();
    }
}

void ToolStateManager::setIsTextEditorActive(bool active)
{
    if (m_isTextEditorActive != active) {
        m_isTextEditorActive = active;
        emit isTextEditorActiveChanged(active);
    }
}

void ToolStateManager::setActiveTextContent(const QString& text)
{
    if (m_activeTextContent != text) {
        m_activeTextContent = text;
        emit activeTextContentChanged(text);
    }
}

void ToolStateManager::setTextInput(const QString& text)
{
    if (m_textInput != text) {
        m_textInput = text;
        emit textInputChanged(text);
    }
}

void ToolStateManager::setFontSize(int size)
{
    if (m_fontSize != size) {
        m_fontSize = size;
        emit fontSizeChanged(size);
        saveSettings();
    }
}

void ToolStateManager::toggleInteractionMode()
{
    if (m_interactionMode == InteractionMode::Drawing) {
        setInteractionMode(InteractionMode::DesktopPassthrough);
    } else {
        setInteractionMode(InteractionMode::Drawing);
    }
}

void ToolStateManager::toggleOrientation()
{
    setIsVerticalLayout(!m_isVerticalLayout);
}

void ToolStateManager::toggleDrawingsVisibility()
{
    setAreDrawingsVisible(!m_areDrawingsVisible);
}

void ToolStateManager::cycleBackgroundMode()
{
    int next = (static_cast<int>(m_backgroundMode) + 1) % 3;
    setBackgroundModeInt(next);
}

void ToolStateManager::undoRequested()
{
    emit requestUndo();
}

void ToolStateManager::redoRequested()
{
    emit requestRedo();
}

void ToolStateManager::clearRequested()
{
    emit requestClear();
}

void ToolStateManager::screenshotRequested()
{
    emit requestScreenshot();
}

void ToolStateManager::toggleToolbar()
{
    setIsToolbarVisible(!m_isToolbarVisible);
}

void ToolStateManager::exitRequested()
{
    saveSettings();
    emit requestExit();
}

} // namespace DrawOnScreen
