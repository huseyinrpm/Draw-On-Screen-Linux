<div align="center">

# 🖊️ DrawOnScreen

**Linux Wayland için Modern, Donanım Hızlandırmalı ve Sıfır Gecikmeli Ekran Çizim & Annotasyon Uygulaması**

*(Native Linux/Wayland Alternative to Epic Pen & Fatih Kalem)*

[![C++20](https://img.shields.io/badge/C++-20-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![Qt6](https://img.shields.io/badge/Qt-6.6+-green.svg?style=flat&logo=qt)](https://www.qt.io/)
[![KDE Frameworks 6](https://img.shields.io/badge/KDE-KF6-5277C3.svg?style=flat&logo=kde)](https://kde.org/)
[![Wayland LayerShell](https://img.shields.io/badge/Wayland-LayerShell-orange.svg?style=flat)](https://wayland.freedesktop.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20(KDE%20Plasma%206%20%7C%20GNOME%20%7C%20Hyprland)-purple.svg?style=flat)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

</div>

---

## 📖 Genel Bakış (Overview)

**DrawOnScreen**, Linux Wayland ortamlarında (KDE Plasma 6, Hyprland, Sway vb.) sunum yaparken, ders anlatırken, video kaydederken veya ekran üzerinde not alırken kullanabileceğiniz **Epic Pen** ve **Fatih Kalem** benzeri güçlü bir ekran üzeri çizim aracıdır.

Doğrudan Wayland **`zwlr_layer_shell_v1`** protokolü ve **Qt6 Quick / C++20** altyapısıyla geliştirilmiştir. Eski X11 çizim araçlarının aksine Wayland üzerinde **sıfır gecikmeyle (0ms latency)**, monitörün tam tazeleme hızında (144Hz+) akıcı çizim ve taşıma imkanı sunar.

---

## ✨ Öne Çıkan Özellikler

- ⚡ **Sıfır Gecikmeli Taşıma & Çizim (0ms Unified Overlay):**
  Araç çubuğu ve çizim motoru tek bir donanım hızlandırmalı Wayland katmanında çalışır. Menüyü farenizle ekranın her yerine anında ve gecikmesiz taşıyabilirsiniz.

- 🖱️ **Kusursuz Masaüstü Modu (Desktop Click-Through):**
  `F9` tuşuyla veya fare butonuna tıklayarak Masaüstü Moduna geçtiğinizde, ekranın tamamı %100 geçirgen (pass-through) hale gelir; arkadaki tarayıcı, terminal ve uygulamalara normal şekilde tıklayabilirsiniz.

- ✍️ **Photoshop Tarzı Ekranda Canlı Metin Yazma:**
  Metin aracını seçip ekranda tıkladığınız yerde doğrudan yanıp sönen dikey imleçle yazmaya başlayabilirsiniz. `Enter` ile onaylayabilir, `Esc` ile iptal edebilirsiniz.

- 👻 **Hayalet Kalem (Ghost Pen / Otomatik Sönen Sunum Kalemi):**
  Öğretmenler ve sunum yapanlar için ideal: Çizilen vurgular veya oklar ekranda kalıcı olmaz; yumuşak bir fade animasyonuyla kendiliğinden kaybolur.

- ⚡ **144Hz Glow Lazer İşaretçi (Laser Pointer):**
  Adaptif kuyruk algoritması ile fare hareketlerinizi takip eden neon lazer izi oluşturur.

- ⬜ **Beyaz Tahta & Kara Tahta Modu:**
  Ders ve toplantılarda tek tıkla **Şeffaf Ekran 🖥️ -> Beyaz Tahta ⬜ -> Kara Tahta ⬛** arasında geçiş yapabilirsiniz.

- 👁️ **Çizimleri ve Tahtayı Anında Gizle/Göster (Eye Toggle):**
  Ekrandaki çizimleri ve tahta arka planını tek bir tıkla anında gizleyip temiz masaüstünü gösterebilir, tekrar tıkladığınızda kaldığınız yerden devam edebilirsiniz.

- 📐 **Geometrik Şekiller ve Oklar:**
  Düz çizgi, dinamik oklu çizgi, dikdörtgen ve daire/elips araçları. İsteğe bağlı **Dolgulu Şekil (Fill)** modu.

- 🎨 **360° Renk Çarkı ve Gelişmiş Ayarlar Çekmecesi:**
  Kromatik renk çemberinden dilediğiniz rengi seçebilir, fırça kalınlığını, lazer süresini ve yazı boyutunu hassas ayarlayabilirsiniz.

- 🔄 **Yatay & Dikey Menü Düzeni:**
  Tek tıkla menüyü yatay veya ekranın yan kenarları için dikey sütun görünümüne döndürebilirsiniz.

- ⌨️ **Tamamen Özelleştirilebilir Kısayol Tuşları:**
  Ayarlar menüsünden tüm kısayol kombinasyonlarını kendi kullanım alışkanlığınıza göre değiştirebilirsiniz.

- 💾 **Ayarların Otomatik Kaydedilmesi:**
  Menüyü ekranın neresine bıraktıysanız, son renk/araç seçimleriniz ve kısayollarınız çıkışta `QSettings` ile güvenle saklanır; bir sonraki açılışta aynı şekilde açılır.

- 📱 **Tam Linux Masaüstü Entegrasyonu:**
  512x512'ye kadar yüksek çözünürlüklü ikon seti, `.desktop` başlatıcısı ile KDE Kickoff, GNOME Dash veya KRunner üzerinden tek tıkla başlatılabilir.

---

## ⌨️ Varsayılan Kısayol Tuşları

| Kısayol | İşlev |
|---|---|
| **`F9`** | 🖱️ Çizim Modu / Masaüstü Modu (Tıklama Geçirgenliği) Arasında Geçiş Yap |
| **`Ctrl + Z`** | ↩️ Son Çizimi Geri Al (Undo) |
| **`Ctrl + Y`** | ↪️ Son Çizimi İleri Al (Redo) |
| **`Ctrl + Shift + C`** veya **`C`** | 🗑️ Ekrandaki Tüm Çizimleri Temizle |
| **`Ctrl + Shift + S`** | 📸 Ekran Görüntüsü Al (Çizimlerle Birlikte Panoya Kopyalar) |
| **`Escape`** | 🚫 Masaüstü Moduna Geç / Aktif Metin Çizimini İptal Et |
| **`Alt + F4`** | ❌ Uygulamadan Çık |

*(Tüm kısayollar Ayarlar Çekmecesi > Kısayollar sekmesinden dilediğiniz gibi değiştirilebilir.)*

---

## 📦 Kurulum Gereksinimleri (Dependencies)

Projeyi derlemek için sisteminizde Qt6, LayerShellQt ve KF6 kütüphaneleri bulunmalıdır:

### 🔹 Arch Linux / Manjaro / EndeavourOS / CachyOS
```bash
sudo pacman -S --needed base-devel cmake ninja gcc extra-cmake-modules \
                       qt6-base qt6-declarative qt6-wayland \
                       layer-shell-qt kglobalaccel librsvg
```

### 🔹 Fedora / RHEL / Nobara
```bash
sudo dnf install gcc-c++ cmake ninja-build extra-cmake-modules \
                 qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtwayland-devel \
                 layer-shell-qt-devel kf6-kglobalaccel-devel librsvg2-tools
```

### 🔹 Ubuntu 24.04+ / Debian 13+
```bash
sudo apt update
sudo apt install build-essential cmake ninja-build extra-cmake-modules \
                 qt6-base-dev qt6-declarative-dev qt6-wayland \
                 liblayershellqt-dev libkf6globalaccel-dev librsvg2-bin
```

---

## 🚀 Hızlı Kurulum (Tek Tıkla)

Depoyu klonlayıp kurulum betiğini çalıştırmanız yeterlidir:

```bash
# 1. Depoyu klonlayın
git clone https://github.com/huseyinrpm/Draw-On-Screen-Linux.git
cd Draw-On-Screen-Linux

# 2. Kurulum betiğini çalıştırın (Derler, ikonları üretir ve sisteme ekler)
./install.sh
```

Kurulum tamamlandıktan sonra:
- **Başlat Menüsünden:** `DrawOnScreen` yazarak açabilirsiniz.
- **KRunner (Alt + Space):** `drawonscreen` yazarak başlatabilirsiniz.
- **Terminalden:** Doğrudan `drawonscreen` komutuyla çalıştırabilirsiniz.

---

## 🛠️ Manuel Derleme (Geliştiriciler İçin)

```bash
# 1. CMake ve Ninja ile derleyin
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 2. Testleri çalıştırın
ctest --test-dir build --output-on-failure

# 3. Yerel olarak çalıştırın
./build/drawonscreen
```

---

## 🗑️ Kaldırma (Uninstallation)

Uygulamayı sisteminizden tamamen kaldırmak için:
```bash
./uninstall.sh
```

---

## 🏗️ Mimari & Dosya Yapısı

```
DrawOnScreen/
├── CMakeLists.txt              # Ana derleme yapılandırması
├── install.sh                  # Tek tıkla native kurulum betiği
├── uninstall.sh                # Sistemden kaldırma betiği
├── resources/
│   ├── drawonscreen.desktop   # Standart XDG masaüstü başlatıcı
│   ├── icons/
│   │   └── drawonscreen.svg   # 512x512 yüksek çözünürlüklü vektörel logo
│   └── resources.qrc          # Qt kaynak dosyası
├── src/
│   ├── main.cpp                # Uygulama başlangıcı ve Wayland ortam hazırlığı
│   ├── core/
│   │   ├── types.h             # Temel veri yapıları (Stroke, ToolType, InteractionMode)
│   │   ├── document.h/.cpp     # Vektörel çizim belgesi, geri/ileri al motoru
│   │   ├── smoother.h/.cpp     # Catmull-Rom Bézier eğri yumuşatma algoritması
│   │   ├── laserengine.h/.cpp  # 144Hz adaptif neon lazer motoru
│   │   └── toolstatemanager.h/.cpp # Araç durumları, kalıcı QSettings yönetimi
│   ├── ui/
│   │   ├── canvasitem.h/.cpp   # QQuickPaintedItem donanım hızlandırmalı çizim motoru
│   │   └── overlaywindow.h/.cpp# Full-Screen Wayland LayerShell Overlay penceresi
│   ├── qml/
│   │   ├── Overlay.qml         # Çizim tuvali ve araç çubuğu ana bileşeni
│   │   ├── Toolbar.qml         # Yatay/dikey kaydırılabilir araç çubuğu
│   │   ├── SettingsDrawer.qml  # 360° Renk çarkı, efekt ve kısayol ayarları paneli
│   │   ├── IconButton.qml      # Vektörel ikon butonları
│   │   ├── ColorPalette.qml    # Renk paleti
│   │   └── SizeSlider.qml      # Fırça kalınlığı kaydırıcısı
│   └── system/
│       ├── shortcutmanager.h/.cpp # KF6GlobalAccel küresel kısayol entegrasyonu
│       └── screenshotmanager.h/.cpp# DBus tabanlı ekran görüntüsü alma motoru
└── tests/
    └── test_core.cpp           # Çekirdek motor birim testleri (Unit Tests)
```

---

## 🤝 Katkıda Bulunma (Contributing)

1. Bu depoyu Fork'layın (`git fork`).
2. Yeni özellik dalı açın (`git checkout -b feature/harika-ozellik`).
3. Değişikliklerinizi commit edin (`git commit -m 'feat: Yeni harika özellik eklendi'`).
4. Dalınıza push yapın (`git push origin feature/harika-ozellik`).
5. Bir **Pull Request (PR)** açın.

---

## 📄 Lisans

Bu proje **MIT Lisansı** ile lisanslanmıştır. Ayrıntılar için `LICENSE` dosyasına bakabilirsiniz.
