#pragma once

#include <QObject>
#include <QAction>
#include <QList>

namespace DrawOnScreen {

class ToolStateManager;

class ShortcutManager : public QObject {
    Q_OBJECT
public:
    explicit ShortcutManager(ToolStateManager* stateMgr, QObject* parent = nullptr);
    ~ShortcutManager() override;

public slots:
    void updateShortcuts();

private:
    void registerAction(QAction* action, const QString& actionName, const QString& displayName, const QKeySequence& defaultSeq);

    ToolStateManager* m_stateMgr = nullptr;
    QAction* m_toggleModeAction = nullptr;
    QAction* m_clearAction = nullptr;
    QAction* m_screenshotAction = nullptr;
    QAction* m_undoAction = nullptr;
    QAction* m_redoAction = nullptr;
    QList<QAction*> m_actions;
};

} // namespace DrawOnScreen
