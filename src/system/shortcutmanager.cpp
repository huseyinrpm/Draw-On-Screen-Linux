#include "shortcutmanager.h"
#include "../core/toolstatemanager.h"
#include <KGlobalAccel>
#include <QKeySequence>
#include <QDebug>

namespace DrawOnScreen {

ShortcutManager::ShortcutManager(ToolStateManager* stateMgr, QObject* parent)
    : QObject(parent)
    , m_stateMgr(stateMgr)
{
    // 1. Toggle Drawing / Desktop Passthrough mode
    m_toggleModeAction = new QAction(this);
    connect(m_toggleModeAction, &QAction::triggered, m_stateMgr, &ToolStateManager::toggleInteractionMode);
    registerAction(m_toggleModeAction, QStringLiteral("toggle_interaction_mode"),
                   QStringLiteral("Çizim / Masaüstü Modunu Aç/Kapat"), QKeySequence(m_stateMgr->shortcutToggleMode()));

    // 2. Clear canvas
    m_clearAction = new QAction(this);
    connect(m_clearAction, &QAction::triggered, m_stateMgr, &ToolStateManager::clearRequested);
    registerAction(m_clearAction, QStringLiteral("clear_canvas"),
                   QStringLiteral("Tüm Çizimleri Temizle"), QKeySequence(m_stateMgr->shortcutClear()));

    // 3. Screenshot and copy
    m_screenshotAction = new QAction(this);
    connect(m_screenshotAction, &QAction::triggered, m_stateMgr, &ToolStateManager::screenshotRequested);
    registerAction(m_screenshotAction, QStringLiteral("capture_screenshot"),
                   QStringLiteral("Ekran Görüntüsü Al ve Kopyala"), QKeySequence(m_stateMgr->shortcutScreenshot()));

    // 4. Undo
    m_undoAction = new QAction(this);
    connect(m_undoAction, &QAction::triggered, m_stateMgr, &ToolStateManager::undoRequested);
    registerAction(m_undoAction, QStringLiteral("undo_stroke"),
                   QStringLiteral("Son Çizimi Geri Al"), QKeySequence(m_stateMgr->shortcutUndo()));

    // 5. Redo
    m_redoAction = new QAction(this);
    connect(m_redoAction, &QAction::triggered, m_stateMgr, &ToolStateManager::redoRequested);
    registerAction(m_redoAction, QStringLiteral("redo_stroke"),
                   QStringLiteral("Son Çizimi İleri Al"), QKeySequence(m_stateMgr->shortcutRedo()));

    connect(m_stateMgr, &ToolStateManager::shortcutsConfigChanged, this, &ShortcutManager::updateShortcuts);
}

ShortcutManager::~ShortcutManager()
{
    for (auto* action : m_actions) {
        KGlobalAccel::self()->removeAllShortcuts(action);
    }
}

void ShortcutManager::registerAction(QAction* action, const QString& actionName, const QString& displayName, const QKeySequence& defaultSeq)
{
    action->setObjectName(actionName);
    action->setText(displayName);
    action->setProperty("componentName", QStringLiteral("drawonscreen"));
    action->setProperty("componentDisplayName", QStringLiteral("DrawOnScreen"));

    KGlobalAccel::self()->setDefaultShortcut(action, {defaultSeq});
    KGlobalAccel::self()->setShortcut(action, {defaultSeq});

    m_actions.append(action);
}

void ShortcutManager::updateShortcuts()
{
    if (m_toggleModeAction) {
        KGlobalAccel::self()->setShortcut(m_toggleModeAction, {QKeySequence(m_stateMgr->shortcutToggleMode())});
    }
    if (m_clearAction) {
        KGlobalAccel::self()->setShortcut(m_clearAction, {QKeySequence(m_stateMgr->shortcutClear())});
    }
    if (m_screenshotAction) {
        KGlobalAccel::self()->setShortcut(m_screenshotAction, {QKeySequence(m_stateMgr->shortcutScreenshot())});
    }
    if (m_undoAction) {
        KGlobalAccel::self()->setShortcut(m_undoAction, {QKeySequence(m_stateMgr->shortcutUndo())});
    }
    if (m_redoAction) {
        KGlobalAccel::self()->setShortcut(m_redoAction, {QKeySequence(m_stateMgr->shortcutRedo())});
    }
}

} // namespace DrawOnScreen
