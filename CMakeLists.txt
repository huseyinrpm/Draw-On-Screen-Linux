cmake_minimum_required(VERSION 3.20)
project(drawonscreen VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Quick
    Qml
    DBus
    WaylandClient
)

find_package(LayerShellQt REQUIRED)
find_package(KF6GlobalAccel REQUIRED)

include(GNUInstallDirs)

set(SOURCES
    src/main.cpp
    src/core/types.h
    src/core/smoother.h
    src/core/smoother.cpp
    src/core/document.h
    src/core/document.cpp
    src/core/laserengine.h
    src/core/laserengine.cpp
    src/core/toolstatemanager.h
    src/core/toolstatemanager.cpp
    src/ui/canvaswindow.h
    src/ui/canvaswindow.cpp
    src/ui/toolbarwindow.h
    src/ui/toolbarwindow.cpp
    src/ui/canvasitem.h
    src/ui/canvasitem.cpp
    src/ui/overlaywindow.h
    src/ui/overlaywindow.cpp
    src/system/shortcutmanager.h
    src/system/shortcutmanager.cpp
    src/system/screenshotmanager.h
    src/system/screenshotmanager.cpp
    resources/resources.qrc
)

add_executable(drawonscreen ${SOURCES})

target_include_directories(drawonscreen PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(drawonscreen PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::Qml
    Qt6::DBus
    Qt6::WaylandClient
    LayerShellQt::Interface
    KF6::GlobalAccel
)

# Installation targets
install(TARGETS drawonscreen
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Test target
enable_testing()
add_executable(test_core
    tests/test_core.cpp
    src/core/smoother.cpp
    src/core/document.cpp
    src/core/laserengine.cpp
    src/core/toolstatemanager.cpp
)
target_include_directories(test_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_core PRIVATE Qt6::Core Qt6::Gui)
add_test(NAME core_tests COMMAND test_core)

install(FILES resources/drawonscreen.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES resources/icons/drawonscreen.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

install(FILES resources/drawonscreen.service
    DESTINATION lib/systemd/user
)
