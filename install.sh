#!/usr/bin/env bash
set -e

# ==============================================================================
# DrawOnScreen - Akıllı Kurulum & Bağımlılık Denetleme Betiği
# Desteklenen Dağıtımlar: Arch/Manjaro/CachyOS, Debian/Ubuntu/Pardus, Fedora/RHEL
# ==============================================================================

echo "🔍 Sistem ve gerekli paketler denetleniyor..."

check_and_install_dependencies() {
    local missing_packages=()
    local distro=""
    local distro_like=""

    if [ -f /etc/os-release ]; then
        # shellcheck disable=SC1091
        . /etc/os-release
        distro="${ID:-}"
        distro_like="${ID_LIKE:-}"
    fi

    # İkon dönüştürme aracı kontrolü
    local has_icon_tool=false
    if command -v rsvg-convert &>/dev/null || command -v magick &>/dev/null || command -v convert &>/dev/null; then
        has_icon_tool=true
    fi

    # 1. Arch Linux / Manjaro / CachyOS / EndeavourOS
    if [[ "$distro" =~ (arch|manjaro|endeavouros|cachyos) ]] || [[ "$distro_like" =~ (arch) ]]; then
        local arch_pkgs=("cmake" "ninja" "gcc" "qt6-base" "qt6-declarative" "qt6-wayland" "layer-shell-qt" "kglobalaccel")
        for pkg in "${arch_pkgs[@]}"; do
            if ! pacman -Q "$pkg" &>/dev/null; then
                missing_packages+=("$pkg")
            fi
        done
        if [ "$has_icon_tool" = false ]; then
            missing_packages+=("librsvg")
        fi

        if [ ${#missing_packages[@]} -ne 0 ]; then
            echo ""
            echo "⚠️  Aşağıdaki gerekli paketler sisteminizde eksik:"
            for p in "${missing_packages[@]}"; do echo "   • $p"; done
            echo ""
            read -r -p "📦 Bu paketleri şimdi otomatik olarak yüklemek istiyor musunuz? [E/h]: " answer
            answer=${answer:-E}
            if [[ "$answer" =~ ^[EeYy]$ || "$answer" == "E" || "$answer" == "e" ]]; then
                echo "⏳ Paketler pacman ile yükleniyor..."
                sudo pacman -S --needed --noconfirm "${missing_packages[@]}"
            else
                echo "❌ Gerekli paketler kurulmadan devam edilemez. Kurulum iptal edildi."
                exit 1
            fi
        fi

    # 2. Debian / Ubuntu / Pardus / Linux Mint / Pop!_OS
    elif [[ "$distro" =~ (ubuntu|debian|pardus|linuxmint|pop) ]] || [[ "$distro_like" =~ (debian|ubuntu) ]]; then
        local deb_pkgs=("build-essential" "cmake" "ninja-build" "qt6-base-dev" "qt6-declarative-dev" "qt6-wayland" "liblayershellqt-dev" "libkf6globalaccel-dev")
        for pkg in "${deb_pkgs[@]}"; do
            if ! dpkg -s "$pkg" &>/dev/null; then
                missing_packages+=("$pkg")
            fi
        done
        if [ "$has_icon_tool" = false ]; then
            missing_packages+=("librsvg2-bin")
        fi

        if [ ${#missing_packages[@]} -ne 0 ]; then
            echo ""
            echo "⚠️  Aşağıdaki gerekli paketler sisteminizde eksik:"
            for p in "${missing_packages[@]}"; do echo "   • $p"; done
            echo ""
            read -r -p "📦 Bu paketleri şimdi otomatik olarak yüklemek istiyor musunuz? [E/h]: " answer
            answer=${answer:-E}
            if [[ "$answer" =~ ^[EeYy]$ || "$answer" == "E" || "$answer" == "e" ]]; then
                echo "⏳ Paketler apt ile yükleniyor..."
                sudo apt-get update
                sudo apt-get install -y "${missing_packages[@]}"
            else
                echo "❌ Gerekli paketler kurulmadan devam edilemez. Kurulum iptal edildi."
                exit 1
            fi
        fi

    # 3. Fedora / RHEL / Nobara / CentOS
    elif [[ "$distro" =~ (fedora|rhel|centos|nobara) ]] || [[ "$distro_like" =~ (fedora|rhel) ]]; then
        local fedora_pkgs=("gcc-c++" "cmake" "ninja-build" "qt6-qtbase-devel" "qt6-qtdeclarative-devel" "qt6-qtwayland-devel" "layer-shell-qt-devel" "kf6-kglobalaccel-devel")
        for pkg in "${fedora_pkgs[@]}"; do
            if ! rpm -q "$pkg" &>/dev/null; then
                missing_packages+=("$pkg")
            fi
        done
        if [ "$has_icon_tool" = false ]; then
            missing_packages+=("librsvg2-tools")
        fi

        if [ ${#missing_packages[@]} -ne 0 ]; then
            echo ""
            echo "⚠️  Aşağıdaki gerekli paketler sisteminizde eksik:"
            for p in "${missing_packages[@]}"; do echo "   • $p"; done
            echo ""
            read -r -p "📦 Bu paketleri şimdi otomatik olarak yüklemek istiyor musunuz? [E/h]: " answer
            answer=${answer:-E}
            if [[ "$answer" =~ ^[EeYy]$ || "$answer" == "E" || "$answer" == "e" ]]; then
                echo "⏳ Paketler dnf ile yükleniyor..."
                sudo dnf install -y "${missing_packages[@]}"
            else
                echo "❌ Gerekli paketler kurulmadan devam edilemez. Kurulum iptal edildi."
                exit 1
            fi
        fi
    fi

    echo "✅ Gerekli tüm bağımlılıklar ve derleme araçları hazır."
}

check_and_install_dependencies

echo ""
echo "🚀 DrawOnScreen Derleniyor ve Sisteme Kuruluyor..."

# 1. Derleme (Ninja ile Hızlı Derleme)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 2. İkon PNG Çözünürlüklerini Oluştur
mkdir -p resources/icons/png
for size in 16 24 32 48 64 128 256 512; do
    if command -v rsvg-convert &>/dev/null; then
        rsvg-convert -w "$size" -h "$size" resources/icons/drawonscreen.svg -o "resources/icons/png/drawonscreen_${size}.png"
    elif command -v magick &>/dev/null; then
        magick -background none -density 300 resources/icons/drawonscreen.svg -resize "${size}x${size}" "resources/icons/png/drawonscreen_${size}.png"
    elif command -v convert &>/dev/null; then
        convert -background none -density 300 resources/icons/drawonscreen.svg -resize "${size}x${size}" "resources/icons/png/drawonscreen_${size}.png"
    fi
done

# 3. Kullanıcı Dizinine (~/.local) Standart XDG Kurulumu
mkdir -p "$HOME/.local/bin"
mkdir -p "$HOME/.local/share/applications"
mkdir -p "$HOME/.local/share/pixmaps"
mkdir -p "$HOME/.local/share/icons/hicolor/scalable/apps"

# Binary dosyasını kopyala
cp -f build/drawonscreen "$HOME/.local/bin/drawonscreen"
chmod +x "$HOME/.local/bin/drawonscreen"

# SVG İkonunu kopyala
cp -f resources/icons/drawonscreen.svg "$HOME/.local/share/icons/hicolor/scalable/apps/drawonscreen.svg"
cp -f resources/icons/drawonscreen.svg "$HOME/.local/share/pixmaps/drawonscreen.svg"

# PNG İkonlarını her boyuta yerleştir
for size in 16 24 32 48 64 128 256 512; do
    if [ -f "resources/icons/png/drawonscreen_${size}.png" ]; then
        mkdir -p "$HOME/.local/share/icons/hicolor/${size}x${size}/apps"
        cp -f "resources/icons/png/drawonscreen_${size}.png" "$HOME/.local/share/icons/hicolor/${size}x${size}/apps/drawonscreen.png"
    fi
done

# 512x512 PNG ikonunu ana pixmaps dizinine de koy
if [ -f "resources/icons/png/drawonscreen_512.png" ]; then
    cp -f "resources/icons/png/drawonscreen_512.png" "$HOME/.local/share/pixmaps/drawonscreen.png"
fi

# Desktop Entry dosyasını oluştur ve kullanıcı dizinine yerleştir
cat << EOF > "$HOME/.local/share/applications/drawonscreen.desktop"
[Desktop Entry]
Name=DrawOnScreen
GenericName=Ekran Çizim Aracı
Comment=Linux Wayland için gelişmiş ekran üzerine çizim ve sunum aracı
Exec=$HOME/.local/bin/drawonscreen
TryExec=$HOME/.local/bin/drawonscreen
Icon=drawonscreen
Terminal=false
Type=Application
Categories=Utility;Graphics;KDE;Qt;
StartupNotify=true
StartupWMClass=drawonscreen
X-KDE-GlobalShortcuts=true
Keywords=draw;screen;annotation;pen;tahta;cizim;epicpen;fatih;
EOF

chmod +x "$HOME/.local/share/applications/drawonscreen.desktop"

# 4. Masaüstü & İkon Önbelleklerini Yenile (KDE Plasma & GNOME için)
echo "🔄 Sistem arayüzü ve ikon önbellekleri güncelleniyor..."
update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true
gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null || true
if command -v kbuildsycoca6 &>/dev/null; then
    kbuildsycoca6 --noincremental 2>/dev/null || true
elif command -v kbuildsycoca5 &>/dev/null; then
    kbuildsycoca5 --noincremental 2>/dev/null || true
fi

echo ""
echo "================================================================="
echo "🎉 TEBRİKLER! DrawOnScreen tam bir yerel (native) uygulama oldu."
echo "📱 Başlat menünüzde (KDE / GNOME / Pardus) logosuyla birlikte görünecektir."
echo "💡 Alt + Boşluk (KRunner) veya Başlat menüsünden 'DrawOnScreen' yazarak açabilirsiniz."
echo "================================================================="
