#!/usr/bin/env bash
set -e

echo "🚀 DrawOnScreen Derleniyor ve Native Uygulama Olarak Sisteme Yükleniyor..."

# 1. Derleme
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 2. İkon PNG Çözünürlüklerini Oluştur
mkdir -p resources/icons/png
for size in 16 24 32 48 64 128 256 512; do
    if command -v rsvg-convert &>/dev/null; then
        rsvg-convert -w $size -h $size resources/icons/drawonscreen.svg -o resources/icons/png/drawonscreen_${size}.png
    elif command -v magick &>/dev/null; then
        magick -background none -density 300 resources/icons/drawonscreen.svg -resize ${size}x${size} resources/icons/png/drawonscreen_${size}.png
    fi
done

# 3. Kullanıcı Dizinine (~/.local) Standart XDG Kurulumu
mkdir -p ~/.local/bin
mkdir -p ~/.local/share/applications
mkdir -p ~/.local/share/pixmaps
mkdir -p ~/.local/share/icons/hicolor/scalable/apps

# Binary dosyasını kopyala
cp -f build/drawonscreen ~/.local/bin/drawonscreen
chmod +x ~/.local/bin/drawonscreen

# SVG İkonunu kopyala
cp -f resources/icons/drawonscreen.svg ~/.local/share/icons/hicolor/scalable/apps/drawonscreen.svg
cp -f resources/icons/drawonscreen.svg ~/.local/share/pixmaps/drawonscreen.svg

# PNG İkonlarını her boyuta yerleştir
for size in 16 24 32 48 64 128 256 512; do
    if [ -f "resources/icons/png/drawonscreen_${size}.png" ]; then
        mkdir -p ~/.local/share/icons/hicolor/${size}x${size}/apps
        cp -f resources/icons/png/drawonscreen_${size}.png ~/.local/share/icons/hicolor/${size}x${size}/apps/drawonscreen.png
    fi
done

# 512x512 PNG ikonunu ana pixmaps dizinine de koy
if [ -f "resources/icons/png/drawonscreen_512.png" ]; then
    cp -f resources/icons/png/drawonscreen_512.png ~/.local/share/pixmaps/drawonscreen.png
fi

# Desktop Entry dosyasını oluştur ve kopyala
cat << 'EOF' > ~/.local/share/applications/drawonscreen.desktop
[Desktop Entry]
Name=DrawOnScreen
GenericName=Ekran Çizim Aracı
Comment=Linux Wayland için gelişmiş ekran üzerine çizim ve sunum aracı
Exec=/home/huseyin/.local/bin/drawonscreen
TryExec=/home/huseyin/.local/bin/drawonscreen
Icon=drawonscreen
Terminal=false
Type=Application
Categories=Utility;Graphics;KDE;Qt;
StartupNotify=true
StartupWMClass=drawonscreen
X-KDE-GlobalShortcuts=true
Keywords=draw;screen;annotation;pen;tahta;cizim;epicpen;
EOF

chmod +x ~/.local/share/applications/drawonscreen.desktop

# 4. Masaüstü & İkon Önbelleklerini Yenile (KDE Plasma & GNOME için)
echo "🔄 Sistem arayüzü ve ikon önbellekleri güncelleniyor..."
update-desktop-database ~/.local/share/applications 2>/dev/null || true
gtk-update-icon-cache -f -t ~/.local/share/icons/hicolor 2>/dev/null || true
if command -v kbuildsycoca6 &>/dev/null; then
    kbuildsycoca6 --noincremental 2>/dev/null || true
elif command -v kbuildsycoca5 &>/dev/null; then
    kbuildsycoca5 --noincremental 2>/dev/null || true
fi

echo ""
echo "================================================================="
echo "🎉 TEBRİKLER! DrawOnScreen tam bir yerel (native) uygulama oldu."
echo "📱 Başlat menünüzde (KDE / GNOME) logosuyla birlikte görünecektir."
echo "💡 Alt + Boşluk (KRunner) veya Başlat menüsünden 'DrawOnScreen' yazarak açabilirsiniz."
echo "================================================================="
