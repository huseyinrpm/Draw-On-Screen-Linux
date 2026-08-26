#pragma once

#include "types.h"
#include <QObject>
#include <QColor>
#include <QString>

namespace DrawOnScreen {

class ToolStateManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(int currentTool READ currentToolInt WRITE setCurrentToolInt NOTIFY currentToolChanged)
    Q_PROPERTY(int interactionMode READ interactionModeInt WRITE setInteractionModeInt NOTIFY interactionModeChanged)
    Q_PROPERTY(int backgroundMode READ backgroundModeInt WRITE setBackgroundModeInt NOTIFY backgroundModeChanged)
    Q_PROPERTY(QColor currentColor READ currentColor WRITE setCurrentColor NOTIFY currentColorChanged)
    Q_PROPERTY(qreal currentWidth READ currentWidth WRITE setCurrentWidth NOTIFY currentWidthChanged)
    Q_PROPERTY(bool isFilled READ isFilled WRITE setIsFilled NOTIFY isFilledChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(bool isToolbarVisible READ isToolbarVisible WRITE setIsToolbarVisible NOTIFY isToolbarVisibleChanged)
    Q_PROPERTY(bool isVerticalLayout READ isVerticalLayout WRITE setIsVerticalLayout NOTIFY isVerticalLayoutChanged)
    Q_PROPERTY(bool areDrawingsVisible READ areDrawingsVisible WRITE setAreDrawingsVisible NOTIFY areDrawingsVisibleChanged)
    Q_PROPERTY(int ghostDurationMs READ ghostDurationMs WRITE setGhostDurationMs NOTIFY ghostDurationMsChanged)
    Q_PROPERTY(int savedToolbarX READ savedToolbarX WRITE setSavedToolbarX NOTIFY savedToolbarXChanged)
    Q_PROPERTY(int savedToolbarY READ savedToolbarY WRITE setSavedToolbarY NOTIFY savedToolbarYChanged)
    Q_PROPERTY(QString textInput READ textInput WRITE setTextInput NOTIFY textInputChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(bool isTextEditorActive READ isTextEditorActive WRITE setIsTextEditorActive NOTIFY isTextEditorActiveChanged)
    Q_PROPERTY(QString activeTextContent READ activeTextContent WRITE setActiveTextContent NOTIFY activeTextContentChanged)

    // Customizable Shortcuts
    Q_PROPERTY(QString shortcutToggleMode READ shortcutToggleMode WRITE setShortcutToggleMode NOTIFY shortcutToggleModeChanged)
    Q_PROPERTY(QString shortcutUndo READ shortcutUndo WRITE setShortcutUndo NOTIFY shortcutUndoChanged)
    Q_PROPERTY(QString shortcutRedo READ shortcutRedo WRITE setShortcutRedo NOTIFY shortcutRedoChanged)
    Q_PROPERTY(QString shortcutClear READ shortcutClear WRITE setShortcutClear NOTIFY shortcutClearChanged)
    Q_PROPERTY(QString shortcutScreenshot READ shortcutScreenshot WRITE setShortcutScreenshot NOTIFY shortcutScreenshotChanged)
    Q_PROPERTY(QString shortcutExit READ shortcutExit WRITE setShortcutExit NOTIFY shortcutExitChanged)

public:
    explicit ToolStateManager(QObject* parent = nullptr);
    ~ToolStateManager() override;

    ToolType currentTool() const { return m_currentTool; }
    int currentToolInt() const { return static_cast<int>(m_currentTool); }
    void setCurrentTool(ToolType tool);
    void setCurrentToolInt(int tool);

    InteractionMode interactionMode() const { return m_interactionMode; }
    int interactionModeInt() const { return static_cast<int>(m_interactionMode); }
    void setInteractionMode(InteractionMode mode);
    void setInteractionModeInt(int mode);

    BackgroundMode backgroundMode() const { return m_backgroundMode; }
    int backgroundModeInt() const { return static_cast<int>(m_backgroundMode); }
    void setBackgroundMode(BackgroundMode mode);
    void setBackgroundModeInt(int mode);

    QColor currentColor() const { return m_currentColor; }
    void setCurrentColor(const QColor& color);

    qreal currentWidth() const { return m_currentWidth; }
    void setCurrentWidth(qreal width);

    bool isFilled() const { return m_isFilled; }
    void setIsFilled(bool filled);

    bool canUndo() const { return m_canUndo; }
    void setCanUndo(bool can);

    bool canRedo() const { return m_canRedo; }
    void setCanRedo(bool can);

    bool isToolbarVisible() const { return m_isToolbarVisible; }
    void setIsToolbarVisible(bool visible);

    bool isVerticalLayout() const { return m_isVerticalLayout; }
    void setIsVerticalLayout(bool vertical);

    bool areDrawingsVisible() const { return m_areDrawingsVisible; }
    void setAreDrawingsVisible(bool visible);

    int ghostDurationMs() const { return m_ghostDurationMs; }
    void setGhostDurationMs(int ms);

    int savedToolbarX() const { return m_savedToolbarX; }
    void setSavedToolbarX(int x);

    int savedToolbarY() const { return m_savedToolbarY; }
    void setSavedToolbarY(int y);

    QString textInput() const { return m_textInput; }
    void setTextInput(const QString& text);

    int fontSize() const { return m_fontSize; }
    void setFontSize(int size);

    bool isTextEditorActive() const { return m_isTextEditorActive; }
    void setIsTextEditorActive(bool active);

    QString activeTextContent() const { return m_activeTextContent; }
    void setActiveTextContent(const QString& text);

    // Shortcuts getters and setters
    QString shortcutToggleMode() const { return m_shortcutToggleMode; }
    void setShortcutToggleMode(const QString& seq);

    QString shortcutUndo() const { return m_shortcutUndo; }
    void setShortcutUndo(const QString& seq);

    QString shortcutRedo() const { return m_shortcutRedo; }
    void setShortcutRedo(const QString& seq);

    QString shortcutClear() const { return m_shortcutClear; }
    void setShortcutClear(const QString& seq);

    QString shortcutScreenshot() const { return m_shortcutScreenshot; }
    void setShortcutScreenshot(const QString& seq);

    QString shortcutExit() const { return m_shortcutExit; }
    void setShortcutExit(const QString& seq);

    // Q_INVOKABLE methods for QML UI
    Q_INVOKABLE void selectTool(int tool) { setCurrentToolInt(tool); }
    Q_INVOKABLE void selectColor(const QColor& color);
    Q_INVOKABLE void selectWidth(qreal width) { setCurrentWidth(width); }
    Q_INVOKABLE void toggleInteractionMode();
    Q_INVOKABLE void setInteractionModeEnum(int mode) { setInteractionModeInt(mode); }
    Q_INVOKABLE void toggleOrientation();
    Q_INVOKABLE void toggleDrawingsVisibility();
    Q_INVOKABLE void setGhostDuration(int ms) { setGhostDurationMs(ms); }
    Q_INVOKABLE void cycleBackgroundMode();
    Q_INVOKABLE void setBackgroundModeEnum(int mode) { setBackgroundModeInt(mode); }
    Q_INVOKABLE void saveToolbarPosition(int x, int y);
    Q_INVOKABLE void setCustomShortcut(const QString& action, const QString& keySeq);
    Q_INVOKABLE void resetShortcutsToDefaults();
    Q_INVOKABLE void saveSettings();
    Q_INVOKABLE void loadSettings();

    Q_INVOKABLE void undoRequested();
    Q_INVOKABLE void redoRequested();
    Q_INVOKABLE void clearRequested();
    Q_INVOKABLE void screenshotRequested();
    Q_INVOKABLE void toggleToolbar();
    Q_INVOKABLE void exitRequested();

signals:
    void currentToolChanged(int tool);
    void interactionModeChanged(int mode);
    void backgroundModeChanged(int mode);
    void currentColorChanged(const QColor& color);
    void currentWidthChanged(qreal width);
    void isFilledChanged(bool filled);
    void canUndoChanged(bool can);
    void canRedoChanged(bool can);
    void isToolbarVisibleChanged(bool visible);
    void isVerticalLayoutChanged(bool vertical);
    void areDrawingsVisibleChanged(bool visible);
    void ghostDurationMsChanged(int ms);
    void savedToolbarXChanged(int x);
    void savedToolbarYChanged(int y);
    void textInputChanged(const QString& text);
    void fontSizeChanged(int size);
    void isTextEditorActiveChanged(bool active);
    void activeTextContentChanged(const QString& text);

    void shortcutToggleModeChanged(const QString& seq);
    void shortcutUndoChanged(const QString& seq);
    void shortcutRedoChanged(const QString& seq);
    void shortcutClearChanged(const QString& seq);
    void shortcutScreenshotChanged(const QString& seq);
    void shortcutExitChanged(const QString& seq);
    void shortcutsConfigChanged();

    void requestUndo();
    void requestRedo();
    void requestClear();
    void requestScreenshot();
    void requestExit();

private:
    ToolType m_currentTool = ToolType::Pen;
    InteractionMode m_interactionMode = InteractionMode::Drawing;
    BackgroundMode m_backgroundMode = BackgroundMode::Transparent;
    QColor m_currentColor = QColor(239, 68, 68);
    qreal m_currentWidth = 4.0;
    bool m_isFilled = false;
    bool m_canUndo = false;
    bool m_canRedo = false;
    bool m_isToolbarVisible = true;
    bool m_isVerticalLayout = false;
    bool m_areDrawingsVisible = true;
    int m_ghostDurationMs = 3000;
    int m_savedToolbarX = 80;
    int m_savedToolbarY = 80;
    bool m_isTextEditorActive = false;
    QString m_activeTextContent;
    QString m_textInput = QStringLiteral("Metin");
    int m_fontSize = 28;

    // Default Shortcuts
    QString m_shortcutToggleMode = QStringLiteral("F9");
    QString m_shortcutUndo = QStringLiteral("Ctrl+Z");
    QString m_shortcutRedo = QStringLiteral("Ctrl+Y");
    QString m_shortcutClear = QStringLiteral("Ctrl+Shift+C");
    QString m_shortcutScreenshot = QStringLiteral("Ctrl+Shift+S");
    QString m_shortcutExit = QStringLiteral("Alt+F4");
};

} // namespace DrawOnScreen
