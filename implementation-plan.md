# Epic Pen / Fatih Kalem Benzeri Ekran Annotasyon Uygulaması
## KDE Plasma 6 (KWin Wayland) Native Implementation Plan

**Hedef:** Arch/CachyOS + KDE Plasma 6 (KWin Wayland) üzerinde native, düşük gecikmeli, tablet destekli ekran çizim/annotasyon aracı.

---

## 1. Teknoloji Yığını ve Mimari Tercih

### 1.1 Karar: C++20 + Qt6 (Widgets/QML hibrit) + `layer-shell-qt` + QPainter (RHI fallback)

| Katman | Seçim | Gerekçe |
|---|---|---|
| Dil | **C++20** | KDE Frameworks 6 ve `layer-shell-qt` zaten C++; Rust bindings (rustc + cxx-qt) ek soyutlama katmanı ve daha ince belgelenmiş Wayland protokol entegrasyonu getirir. Üretim hızı ve KWin kaynak koduyla uyum için C++ tercih edilir. |
| UI Framework | **Qt6 Widgets** (overlay canvas) + **QML/Quick** (floating toolbar) | Widgets: `QPainter` üzerinden raster çizim döngüsü daha basit, deterministik ve düşük gecikmelidir (immediate mode). QML/Quick: Toolbar'da modern/akışkan animasyonlar, blur/şeffaflık efektleri için idealdir (Qt Quick Scene Graph, GPU'da render edilir). |
| Wayland Katman Entegrasyonu | **layer-shell-qt** (KDE Frameworks) | `zwlr_layer_shell_v1` protokolünün Qt sarmalayıcısı. KWin bu protokolü native destekler (`org_kde_plasma_*` değil, tam wlroots-uyumlu protokol). CMake ile `LayerShellQt::Interface` linklenir. |
| Render Backend | **QPainter (raster) — varsayılan**, gerekirse **QRhi (Vulkan/OpenGL)** | Kalem/fırça vuruşu gibi ince, sık-güncellenen ama düşük poligon sayılı içerikte QPainter + `QOpenGLWidget`/`QRasterWindow` gecikme açısından yeterli (<5ms per-frame paint). Yalnızca lazer işaretçi parçacık efektleri veya çok katmanlı blend gerektiğinde `QQuickWindow` + Scene Graph (RHI, otomatik Vulkan/OpenGL seçimi) devreye alınır. |
| Tablet Girdisi | `QTabletEvent` (Qt6 native) | Qt6, `libinput` üzerinden gelen basınç/eğim/rotation verisini doğrudan `QTabletEvent::pressure()`, `::xTilt()`, `::yTilt()` olarak sunar; ek bir libinput bağımlılığı gerekmez. |
| Global Kısayollar | `KGlobalAccel` (KDE Frameworks) **birincil**, `xdg-desktop-portal` GlobalShortcuts **fallback** | KGlobalAccel, Plasma oturumunda doğrudan DBus üzerinden `kglobalaccel5/6` servisine kayıt olur ve KWin ile çakışmadan çalışır. Portal API'si yalnızca sandboxed (Flatpak) dağıtım senaryosu için yedek olarak planlanır. |
| Ekran Görüntüsü | `org.kde.KWin.ScreenShot2` DBus arayüzü veya `xdg-desktop-portal` `Screenshot` metodu | Spectacle'ın kendisi de bu DBus arayüzünü kullanır; doğrudan entegre ederek Spectacle'a proses bağımlılığı olmadan native yakalama yapılır. |
| Build Sistemi | **CMake + ECM (extra-cmake-modules)** | KDE ekosistemünün standart build sistemi; `find_package(LayerShellQt)`, `find_package(KF6GlobalAccel)` gibi modüller ECM üzerinden gelir. |

### 1.2 Neden Bu Stack En Kararlı ve Düşük Gecikmeli?

- **Immediate-mode raster çizim (QPainter)**: Overlay canvas'ta her `tabletEvent`/`mouseMoveEvent` sonrası yalnızca "dirty rect" (değişen bölge) yeniden çizilir → tam ekran yeniden render maliyeti yok.
- **layer-shell-qt**, KWin'in compositor thread'i ile doğrudan protokol seviyesinde konuşur; XWayland köprüsü (X11 uyumluluk katmanı) devre dışı kalır, bu da input gecikmesini ~1 frame'e (compositor vsync) indirir.
- **Qt6'nın Wayland QPA plugin'i**, `wl_surface.damage_buffer` ile yalnızca değişen piksel bölgesini compositor'a bildirir; bu da GPU bant genişliği tasarrufu sağlar.
- Rust alternatifi (`smithay-client-toolkit` + `iced`/`egui`) daha "bare-metal" olsa da, KWin'in resmi/test edilen path'i Qt/KDE Frameworks olduğundan, protokol uyumsuzluk riski (özellikle input-region ve layer anchor davranışlarında) çok daha düşüktür.

### 1.3 Mimari Şema (Yüksek Seviye)

```
┌─────────────────────────────────────────────────────────────────┐
│                        KWin (Wayland Compositor)                  │
│  ┌───────────────┐   ┌────────────────────┐   ┌───────────────┐  │
│  │  Layer::Overlay│   │  Layer::Overlay     │   │ Normal Windows│  │
│  │  Canvas Surface│   │  Toolbar Surface    │   │ (arka plan)   │  │
│  └───────┬───────┘   └─────────┬──────────┘   └───────────────┘  │
└──────────┼─────────────────────┼──────────────────────────────────┘
           │ zwlr_layer_shell_v1 │ zwlr_layer_shell_v1
           │                     │
┌──────────▼─────────────────────▼──────────────────────────────────┐
│                    Uygulama Süreci (tek process, çoklu pencere)     │
│                                                                      │
│  ┌────────────────────┐        ┌───────────────────────────────┐  │
│  │  CanvasWindow       │        │  ToolbarWindow (QML)           │  │
│  │  (QRasterWindow/    │◄──────►│  - Mode Manager                │  │
│  │   QOpenGLWindow)    │ Signal │  - Tool/Color/Width State       │  │
│  │  - StrokeEngine     │ /Slot  │  - Drag & Snap                  │  │
│  │  - TabletInput      │        └───────────────────────────────┘  │
│  │  - UndoStack        │                                            │
│  │  - InputRegionMgr   │        ┌───────────────────────────────┐  │
│  │    (pass-through)   │        │  GlobalShortcutManager          │  │
│  └─────────┬───────────┘        │  (KGlobalAccel)                 │  │
│            │                     └───────────────────────────────┘  │
│  ┌─────────▼───────────┐        ┌───────────────────────────────┐  │
│  │ RenderPipeline       │        │ ScreenshotService                │  │
│  │ (QPainter / QRhi)    │        │ (org.kde.KWin.ScreenShot2 DBus)  │  │
│  └──────────────────────┘        └───────────────────────────────┘  │
└──────────────────────────────────────────────────────────────────┘
```

---

## 2. KWin & Wayland Katman Yönetimi

### 2.1 Layer-Shell Stratejisi

İki ayrı `LayerShellQt::Window` örneği kullanılır:

| Pencere | Layer | Anchor | Keyboard Interactivity | Exclusive Zone |
|---|---|---|---|---|
| **CanvasWindow** (tam ekran overlay) | `Overlay` | Top+Bottom+Left+Right (tüm ekran) | `OnDemand` (yalnızca çizim modunda) | `0` |
| **ToolbarWindow** (floating araç çubuğu) | `Overlay` | Yok (serbest konumlandırma, `moveWindow` ile) | `Exclusive` (her zaman tıklanabilir) | `-1` (auto) |

`Layer::Overlay` seçilmesinin nedeni: KWin'de bu katman, tüm normal ve "always-on-top" pencerelerin **üzerinde** render edilir — yalnızca lock screen ve OSD gibi sistem bileşenleri üzerinde kalabilir. Bu, annotasyonun her uygulamanın üzerinde görünmesini garanti eder.

```cpp
// CanvasWindow kurulumu (basitleştirilmiş)
#include <LayerShellQt/window.h>

void CanvasWindow::configureLayerShell()
{
    auto layerWindow = LayerShellQt::Window::get(this->windowHandle());
    layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
    layerWindow->setAnchors(
        LayerShellQt::Window::AnchorTop   |
        LayerShellQt::Window::AnchorBottom|
        LayerShellQt::Window::AnchorLeft  |
        LayerShellQt::Window::AnchorRight);
    layerWindow->setExclusiveZone(0);
    layerWindow->setKeyboardInteractivity(
        LayerShellQt::Window::KeyboardInteractivityOnDemand);
    layerWindow->setScope(QStringLiteral("screen-annotation-canvas"));
}
```

### 2.2 Input Pass-Through (Etkileşim Modu) Mekanizması

İki mod arasında geçiş, **wl_surface input region** manipülasyonu ile sağlanır:

- **Çizim Modu**: `setMask()` çağrılmaz / input region = tüm yüzey → tüm fare/kalem olayları uygulamaya gider.
- **Masaüstü Etkileşim Modu**: `QWindow::setMask(QRegion())` (boş bölge) çağrılır → Qt bunu Wayland QPA katmanında `wl_surface.set_input_region` ile boş bir `wl_region`'a çevirir → compositor tüm input olaylarını **doğrudan altındaki pencereye** iletir, üstteki overlay tamamen "hayalet" (click-through) hâle gelir.

```cpp
void CanvasWindow::setInteractionMode(InteractionMode mode)
{
    if (mode == InteractionMode::DesktopPassthrough) {
        // Boş region => input tamamen alttaki pencerelere geçer
        setMask(QRegion());
        m_layerWindow->setKeyboardInteractivity(
            LayerShellQt::Window::KeyboardInteractivityNone);
    } else { // DrawingMode
        // Tüm pencere alanı input alır
        setMask(QRegion(rect()));
        m_layerWindow->setKeyboardInteractivity(
            LayerShellQt::Window::KeyboardInteractivityOnDemand);
    }
    m_currentMode = mode;
    update();
}
```

> **Not:** Toolbar penceresi her zaman kendi küçük dikdörtgen bölgesinde input alır (asla pass-through olmaz); yalnızca CanvasWindow'un input region'ı değişir. Bu sayede kullanıcı Etkileşim Modu'ndayken bile toolbar'a tıklayıp Çizim Modu'na geri dönebilir.

### 2.3 Kısmi Pass-Through (Hibrit Mod) — İleri Seviye

Çizilmiş nesnelerin (örn. bir ok veya metin kutusu) üzerine tıklanabilir "seçim" davranışı istenirse, `setMask()` yalnızca çizim bulunan piksel bölgeleriyle sınırlandırılabilir (QRegion birleşimi ile stroke bounding box'ları toplanır). Bu, "tamamen şeffaf ama çizili alanlar tıklanabilir" davranışı sağlar — Phase 4 sonrası opsiyonel geliştirme.

---

## 3. Grafik Pipeline & Fırça Motoru

### 3.1 Çizim Verisi Saklama Modeli

Vektörel path tabanlı model tercih edilir (piksel tabanlı yalnızca "piksel silgi" için yardımcı katman olarak kullanılır):

```cpp
struct StrokePoint {
    QPointF pos;
    qreal pressure;   // 0.0 - 1.0
    qreal tiltX, tiltY;
    qint64 timestampMs;
};

struct Stroke {
    QUuid id;
    ToolType tool;          // Pen, Highlighter, Eraser, Laser
    QColor color;            // RGBA
    qreal baseWidth;
    QVector<StrokePoint> points;
    QPainterPath cachedPath;  // Catmull-Rom -> Bezier dönüşümü sonrası cache
    bool isDirty = true;      // path yeniden hesaplanmalı mı?
};

class CanvasDocument {
public:
    void addPoint(const StrokePoint& p);          // aktif stroke'a ekle
    void commitStroke();                            // stroke'u tamamla, undo stack'e it
    void undo();
    void redo();
    void clear();
    const QVector<Stroke>& strokes() const;
private:
    QVector<Stroke> m_strokes;
    QVector<Stroke> m_redoBuffer;   // redo için ayrı buffer (bellek limitiyle sınırlı)
    Stroke m_activeStroke;
};
```

**Undo/Redo Bellek Yönetimi:**
- Her `commitStroke()` çağrısında stroke, `m_strokes`'a taşınır ve **command pattern** ile bir `AddStrokeCommand` undo stack'e (`QVector<QSharedPointer<Command>>`, max derinlik örn. 100) eklenir.
- Bellek patlamasını önlemek için: (a) stack derinliği sınırlı, (b) her stroke'un `QPainterPath` cache'i yalnızca render anında hesaplanır ve `isDirty` bayrağıyla tembel (lazy) yenilenir, (c) çok uzun stroke'lar (>2000 nokta) belirli bir mesafe eşiğinde otomatik segmentlere bölünür (Douglas-Peucker ile nokta azaltma).

### 3.2 Eğri Yumuşatma (Smoothing) — Bézier Dönüşümü

Ham `StrokePoint` dizisi, **Catmull-Rom spline → kübik Bézier** dönüşümüyle yumuşatılır (her 4 ardışık kontrol noktasından bir Bézier segmenti üretilir):

```cpp
QPainterPath StrokeRenderer::buildSmoothPath(const QVector<StrokePoint>& pts)
{
    QPainterPath path;
    if (pts.size() < 2) return path;
    path.moveTo(pts[0].pos);

    for (int i = 0; i < pts.size() - 1; ++i) {
        QPointF p0 = pts[qMax(0, i - 1)].pos;
        QPointF p1 = pts[i].pos;
        QPointF p2 = pts[i + 1].pos;
        QPointF p3 = pts[qMin(pts.size() - 1, i + 2)].pos;

        // Catmull-Rom -> Bezier kontrol noktaları
        QPointF c1 = p1 + (p2 - p0) / 6.0;
        QPointF c2 = p2 - (p3 - p1) / 6.0;
        path.cubicTo(c1, c2, p2);
    }
    return path;
}
```

Fırça kalınlığı, `pressure` değerine göre segment bazında `QPen::setWidthF()` ile modüle edilir; basınca duyarlı değişken kalınlık için path tek renkte değil, küçük dolgulu poligon şeritleri (variable-width stroke tessellation) olarak da render edilebilir (Phase 2 ileri seviye).

### 3.3 Highlighter (İşaretleyici) — Alfa Harmanlama

```cpp
painter.setCompositionMode(QPainter::CompositionMode_Multiply); // veya SourceOver + düşük alfa
QColor highlighterColor = baseColor;
highlighterColor.setAlphaF(0.35);
painter.setPen(QPen(highlighterColor, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
painter.drawPath(path);
```
`CompositionMode_Multiply` üst üste binen highlighter vuruşlarında gerçek fosforlu kalem görünümü verir (koyulaşma yerine ton karışımı).

### 3.4 Silgi (Eraser) — İki Mod

1. **Vektörel silgi**: Silgi imleci bir stroke'un `QPainterPath`'i ile kesişirse (`QPainterPath::intersects()`), o stroke tamamen veya path-clipping ile kısmen (`QPainterPath::subtracted()`) kaldırılır.
2. **Piksel silgi**: `QPainter::CompositionMode_Clear` ile ayrı bir "eraser layer" (QImage, ARGB32_Premultiplied) üzerine çizilir; final render'da bu katman canvas'tan `DestinationOut` modunda çıkarılır. Bu, serbest-form silme için gereklidir.

### 3.5 Lazer İşaretçi Render Döngüsü

Lazer izi, kalıcı `Stroke` listesine eklenmez; ayrı bir **ring buffer + fade-out timer** ile yönetilir:

```cpp
class LaserPointerEngine : public QObject {
    Q_OBJECT
public:
    explicit LaserPointerEngine(QObject* parent = nullptr) : QObject(parent) {
        connect(&m_frameTimer, &QTimer::timeout, this, &LaserPointerEngine::tick);
        m_frameTimer.setTimerType(Qt::PreciseTimer);
        m_frameTimer.start(1000 / 144); // 144Hz hedef; QScreen::refreshRate() ile dinamik ayarlanabilir
    }

    void addPoint(QPointF pos) {
        m_points.push_back({pos, QDateTime::currentMSecsSinceEpoch(), 1.0});
    }

private slots:
    void tick() {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        constexpr qint64 fadeDurationMs = 500;
        // Süresi dolan noktaları temizle, kalanların opacity'sini yaşa göre hesapla
        for (auto& pt : m_points)
            pt.opacity = qMax(0.0, 1.0 - double(now - pt.timestamp) / fadeDurationMs);
        m_points.erase(std::remove_if(m_points.begin(), m_points.end(),
            [](const auto& p){ return p.opacity <= 0.0; }), m_points.end());
        emit frameReady(); // CanvasWindow::update() tetikler (yalnızca dirty rect)
    }

signals:
    void frameReady();

private:
    struct LaserPoint { QPointF pos; qint64 timestamp; qreal opacity; };
    QVector<LaserPoint> m_points;
    QTimer m_frameTimer;
};
```

`QScreen::refreshRate()` başlangıçta okunarak timer periyodu dinamik ayarlanır; 144Hz+ ekranlarda daha akıcı fade, 60Hz ekranlarda gereksiz CPU kullanımı önlenir. `frameReady` sinyali yalnızca lazer izinin bounding box'ını `update(QRect)` ile invalidate eder — tüm canvas yeniden çizilmez.

---

## 4. Adım Adım Yol Haritası

### **Phase 1 — Minimal KWin Wayland Overlay & Pass-through PoC**
- [ ] CMake iskeleti: ECM, `Qt6::Widgets`, `Qt6::WaylandClient`, `LayerShellQt::Interface`, `KF6::GlobalAccel`
- [ ] `QRasterWindow` tabanlı tam ekran şeffaf pencere (`Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint`, `setAttribute(Qt::WA_TranslucentBackground)`)
- [ ] `layer-shell-qt` ile `Layer::Overlay` bağlama, tüm kenarlara anchor
- [ ] `setMask()` ile pass-through / interactive mod geçiş **tek tuşla** (örn. `Escape` veya global kısayol) test edilmesi
- [ ] Çıktı: `kwin_wayland` altında (nested test compositor `weston` veya gerçek oturumda) şeffaf overlay'in görünürlüğü ve click-through doğrulaması
- [ ] Test senaryosu: overlay açıkken arka planda bir terminal/tarayıcıya tıklanabildiğinin manuel doğrulanması

### **Phase 2 — Grafik Tablet & Çizim Motoru**
- [ ] `QTabletEvent` handler'ları (`tabletEvent()` override): `TabletPress`, `TabletMove`, `TabletRelease`
- [ ] `pressure()`, `xTilt()`, `yTilt()` verisinin `StrokePoint`'e aktarılması
- [ ] Fare (mouse) fallback: tablet yoksa sabit basınç (1.0) ile `mousePressEvent`/`mouseMoveEvent`
- [ ] Catmull-Rom → Bézier smoothing implementasyonu (bkz. §3.2)
- [ ] `CanvasDocument` + Undo/Redo command stack
- [ ] Çoklu katman desteği (opsiyonel: her stroke grubu ayrı `QImage` katmanına render, performans testi sonrası tek katmana indirgenebilir)
- [ ] Performans hedefi: 144Hz ekranda stroke başına <7ms paint süresi (profiling: `QElapsedTimer` + `perf`)

### **Phase 3 — Floating UI & Toolbar State Manager**
- [ ] QML `ApplicationWindow` (frameless, `LayerShellQt::Window` ile `Overlay` katmanına bağlı, serbest sürüklenebilir)
- [ ] Sürükleme: `MouseArea` + `DragHandler`, konum `QSettings` ile kalıcı hâle getirilir
- [ ] `ToolStateManager` (C++ QObject, `Q_PROPERTY` ile QML'e expose): aktif araç, renk, kalınlık, mod
- [ ] Kenara yapışma (edge-snapping) ve minimize/genişletme animasyonları (`Behavior on x/y`, `NumberAnimation`)
- [ ] Renk paleti: RGBA slider'lar (`Qt Quick Controls 2` `ColorDialog` veya özel QML bileşeni)
- [ ] Signal/slot köprüsü: Toolbar seçimleri `CanvasWindow`'a `Q_INVOKABLE` metodlarla iletilir

### **Phase 4 — Özel Araçlar (Lazer, Silgi, Ekran Görüntüsü)**
- [ ] `LaserPointerEngine` entegrasyonu (bkz. §3.5)
- [ ] Vektörel + piksel silgi implementasyonu (bkz. §3.4)
- [ ] `org.kde.KWin.ScreenShot2` DBus arayüzü ile native ekran görüntüsü yakalama:
  ```cpp
  auto msg = QDBusMessage::createMethodCall(
      "org.kde.KWin", "/org/kde/KWin/ScreenShot2",
      "org.kde.KWin.ScreenShot2", "CaptureActiveScreen");
  // Reply: UnixFD (dmabuf) veya PipeWire stream -> QImage'a dönüştürülür
  ```
- [ ] Annotasyonlu görüntüyü PNG olarak diske kaydetme + panoya (clipboard) kopyalama (`QGuiApplication::clipboard()->setPixmap()`)
- [ ] `KGlobalAccel` ile global kısayollar: Çizim modu aç/kapa, hızlı ekran görüntüsü, undo/redo, temizle

### **Phase 5 — Arch/CachyOS Paketleme**
- [ ] `CMakeLists.txt` finalizasyonu: `install()` kuralları (binary, `.desktop` dosyası, ikonlar)
- [ ] `PKGBUILD` yazımı (aşağıda örnek)
- [ ] Systemd **user service** (opsiyonel, oturum açılışında arka planda başlatma / global kısayol tetikleyici olarak)
- [ ] `.desktop` dosyası (Plasma uygulama menüsü + `StartupWMClass`)
- [ ] AUR'a yayınlama (`makepkg --printsrcinfo > .SRCINFO`, git repo)

```bash
# PKGBUILD (özet iskelet)
pkgname=screen-annotate-kde
pkgver=0.1.0
pkgrel=1
pkgdesc="KDE Plasma 6 Wayland için donanım hızlandırmalı ekran annotasyon aracı"
arch=('x86_64')
url="https://github.com/kullanici/screen-annotate-kde"
license=('GPL-3.0-or-later')
depends=('qt6-base' 'qt6-declarative' 'kwindowsystem6' 'kglobalaccel6' 'layer-shell-qt')
makedepends=('cmake' 'extra-cmake-modules' 'qt6-tools')
source=("$pkgname-$pkgver.tar.gz::https://github.com/kullanici/screen-annotate-kde/archive/v$pkgver.tar.gz")
sha256sums=('SKIP')

build() {
  cmake -B build -S "$pkgname-$pkgver" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
  cmake --build build
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}
```

---

## 5. Kritik PoC Kod Örneği

Aşağıdaki örnek: şeffaf, `Layer::Overlay` katmanına bağlı, tablet basıncını yakalayan ve **F9** tuşuyla input pass-through moduna geçen minimal çalışan bir `CanvasWindow`.

### 5.1 `CMakeLists.txt` (PoC)

```cmake
cmake_minimum_required(VERSION 3.20)
project(overlay_poc VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets WaylandClient)
find_package(LayerShellQt REQUIRED)

add_executable(overlay_poc main.cpp canvaswindow.cpp)
target_link_libraries(overlay_poc PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::WaylandClient
    LayerShellQt::Interface
)
```

### 5.2 `canvaswindow.h`

```cpp
#pragma once
#include <QRasterWindow>
#include <QPainterPath>
#include <QVector>

class CanvasWindow : public QRasterWindow {
    Q_OBJECT
public:
    explicit CanvasWindow();

protected:
    void paintEvent(QPaintEvent*) override;
    void tabletEvent(QTabletEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void keyPressEvent(QKeyEvent* ev) override;
    bool event(QEvent* ev) override; // Wayland surface hazır olduğunda layer-shell kur

private:
    void configureLayerShell();
    void togglePassthrough();

    QVector<QPainterPath> m_strokes;
    QPainterPath m_activePath;
    bool m_drawing = false;
    bool m_passthrough = false;
    qreal m_lastPressure = 1.0;
};
```

### 5.3 `canvaswindow.cpp`

```cpp
#include "canvaswindow.h"
#include <LayerShellQt/window.h>
#include <QPainter>
#include <QTabletEvent>
#include <QKeyEvent>
#include <QGuiApplication>
#include <QScreen>

CanvasWindow::CanvasWindow()
{
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // Wayland QPA: şeffaf arka plan için surface format ayarı
    QSurfaceFormat fmt = format();
    fmt.setAlphaBufferSize(8);
    setFormat(fmt);

    // Ekran boyutuna tam otur
    if (auto* scr = QGuiApplication::primaryScreen())
        setGeometry(scr->geometry());

    setTitle(QStringLiteral("screen-annotate-poc"));
}

bool CanvasWindow::event(QEvent* ev)
{
    // PlatformSurface oluşturulduğunda layer-shell konfigürasyonu yapılmalı
    if (ev->type() == QEvent::PlatformSurface) {
        auto* pse = static_cast<QPlatformSurfaceEvent*>(ev);
        if (pse->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
            configureLayerShell();
        }
    }
    return QRasterWindow::event(ev);
}

void CanvasWindow::configureLayerShell()
{
    auto* layerWindow = LayerShellQt::Window::get(this);
    if (!layerWindow) return; // Wayland dışı platformda (X11) no-op

    layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
    layerWindow->setAnchors(
        LayerShellQt::Window::AnchorTop   |
        LayerShellQt::Window::AnchorBottom|
        LayerShellQt::Window::AnchorLeft  |
        LayerShellQt::Window::AnchorRight);
    layerWindow->setExclusiveZone(-1);
    layerWindow->setKeyboardInteractivity(
        LayerShellQt::Window::KeyboardInteractivityOnDemand);
    layerWindow->setScope(QStringLiteral("screen-annotation-canvas"));
}

void CanvasWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), Qt::transparent); // tam şeffaf arka plan

    QPen pen(Qt::red, 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);

    for (const auto& path : m_strokes)
        p.drawPath(path);

    if (m_drawing) {
        pen.setWidthF(3.0 * qMax(0.15, m_lastPressure)); // basınca duyarlı kalınlık
        p.setPen(pen);
        p.drawPath(m_activePath);
    }

    // Pass-through modunda küçük bir durum göstergesi (sağ üst köşe)
    p.setPen(m_passthrough ? Qt::green : Qt::red);
    p.drawText(QPointF(20, 30),
        m_passthrough ? QStringLiteral("PASSTHROUGH (F9 ile geri dön)")
                       : QStringLiteral("ÇİZİM MODU (F9: passthrough)"));
}

void CanvasWindow::tabletEvent(QTabletEvent* ev)
{
    m_lastPressure = ev->pressure();
    switch (ev->type()) {
    case QEvent::TabletPress:
        m_activePath = QPainterPath(ev->position());
        m_drawing = true;
        break;
    case QEvent::TabletMove:
        if (m_drawing) m_activePath.lineTo(ev->position());
        break;
    case QEvent::TabletRelease:
        if (m_drawing) {
            m_strokes.push_back(m_activePath);
            m_activePath = QPainterPath();
            m_drawing = false;
        }
        break;
    default: break;
    }
    ev->accept();
    update();
}

// Tablet yoksa fare fallback (basınç = 1.0 sabit)
void CanvasWindow::mousePressEvent(QMouseEvent* ev)
{
    if (ev->source() != Qt::MouseEventSynthesizedByQt) { // tablet event zaten synth mouse üretir, çakışmayı önle
        m_activePath = QPainterPath(ev->position());
        m_lastPressure = 1.0;
        m_drawing = true;
    }
}
void CanvasWindow::mouseMoveEvent(QMouseEvent* ev)
{
    if (m_drawing) { m_activePath.lineTo(ev->position()); update(); }
}
void CanvasWindow::mouseReleaseEvent(QMouseEvent*)
{
    if (m_drawing) {
        m_strokes.push_back(m_activePath);
        m_activePath = QPainterPath();
        m_drawing = false;
        update();
    }
}

void CanvasWindow::keyPressEvent(QKeyEvent* ev)
{
    if (ev->key() == Qt::Key_F9) {
        togglePassthrough();
    } else if (ev->key() == Qt::Key_C) {
        m_strokes.clear();
        update();
    }
}

void CanvasWindow::togglePassthrough()
{
    m_passthrough = !m_passthrough;
    if (m_passthrough) {
        setMask(QRegion()); // boş region -> input alttaki pencerelere geçer
    } else {
        setMask(QRegion(0, 0, width(), height()));
    }
    update();
}
```

### 5.4 `main.cpp`

```cpp
#include <QGuiApplication>
#include "canvaswindow.h"

int main(int argc, char** argv)
{
    // Wayland platformunu zorla (X11/XWayland fallback'i engelle)
    qputenv("QT_QPA_PLATFORM", "wayland");

    QGuiApplication app(argc, argv);
    app.setApplicationName("screen-annotate-poc");

    CanvasWindow canvas;
    canvas.showFullScreen();

    return app.exec();
}
```

> **PoC Test Notu:** F9 ile pass-through moduna geçildiğinde, overlay penceresi görünür kalır (çizimler ekranda durur) ama fare tıklamaları artık altındaki uygulamalara ulaşır — bu, `set_input_region` davranışının KWin'de doğru çalıştığının doğrulamasıdır. `C` tuşu tüm çizimi temizler (Phase 4'te `Ctrl+Z`'ye genişletilecek undo stack'in yerini geçici olarak tutar).

### 5.5 Derleme ve Çalıştırma

```bash
# Gerekli paketler (CachyOS/Arch)
sudo pacman -S extra-cmake-modules qt6-base qt6-declarative \
               qt6-wayland layer-shell-qt cmake

cmake -B build -S .
cmake --build build -j$(nproc)
QT_QPA_PLATFORM=wayland ./build/overlay_poc
```

---

## 6. Sonraki Adımlar / Riskler ve Azaltma Stratejileri

| Risk | Azaltma |
|---|---|
| `layer-shell-qt` bazı KWin sürümlerinde `KeyboardInteractivityOnDemand` davranışında tutarsızlık gösterebilir | Phase 1'de hedef KWin sürümüyle (Plasma 6.x) erken entegrasyon testi; gerekirse `xdg-activation` ile odak yönetimi |
| Tablet basınç verisi bazı XP-Pen sürücülerinde `libinput` yerine özel kernel modülü (`digimend`) gerektirebilir | Kurulum dokümantasyonunda `digimend-kernel-driver-dkms-git` (AUR) bağımlılığı not edilir |
| Yüksek Hz ekranlarda lazer efekti CPU kullanımı | `QScreen::refreshRate()` bazlı adaptif timer + dirty-rect optimizasyonu (bkz. §3.5) |
| Çoklu monitör senaryosunda overlay'in her ekranda ayrı `CanvasWindow` gerektirmesi | Phase 2 sonunda `QScreen` listesi üzerinden per-screen `CanvasWindow` instance yönetimi eklenir |

---

*Bu doküman, projenin Phase 1 PoC'sinden Arch/CachyOS paketlemesine kadar olan tüm teknik kararları ve gerekçelerini içerir. Her phase kendi git branch'inde geliştirilip `main`'e merge edilmeden önce ilgili bölümdeki test kriterleri doğrulanmalıdır.*
