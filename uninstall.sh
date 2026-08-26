#!/usr/bin/env bash
set -e

echo "🗑️ DrawOnScreen Sistemden Kaldırılıyor..."

rm -f ~/.local/bin/drawonscreen
rm -f ~/.local/share/applications/drawonscreen.desktop
rm -f ~/.local/share/icons/hicolor/scalable/apps/drawonscreen.svg
rm -f ~/.local/lib/systemd/user/drawonscreen.service

update-desktop-database ~/.local/share/applications 2>/dev/null || true
gtk-update-icon-cache -f -t ~/.local/share/icons/hicolor 2>/dev/null || true
kbuildsycoca6 2>/dev/null || true

echo "✅ DrawOnScreen sistemden başarıyla kaldırıldı."
