# Vendored subset of QLight Widgets (MIT, see LICENSE in this directory),
# copied from qlight-widgets commit 87509c8. Built as a static library that is
# linked straight into the app, so there is no extra DLL to deploy.
#
# Components that the app does not use (NavigationView, Splitter, TabWidget,
# Slider, Switch, GroupBox) and the examples/tests were left out.
#
# FramelessWindow (the custom title bar) needs QWindowKit, which is fetched
# from GitHub at configure time. It is only enabled on Windows, the platform
# it is tested on; macOS and Linux keep the native window frame.

cmake_minimum_required(VERSION 3.21)
project(QLightWidgets VERSION 0.1.0 LANGUAGES CXX)

option(QLIGHT_WITH_WINDOWKIT "Frameless window support (downloads QWindowKit at configure time)" ${WIN32})

find_package(Qt6 6.7 REQUIRED COMPONENTS Widgets Svg)

add_library(qlight-widgets STATIC
    src/Accessibility.cpp
    src/Accessibility.h
    src/Calendar.cpp
    src/Card.cpp
    src/CheckBox.cpp
    src/ColorAnimator.cpp
    src/ColorAnimator.h
    src/ComboBox.cpp
    src/Dialog.cpp
    src/Field.h
    src/FluentIcon.cpp
    src/Flyout.cpp
    src/InfoBar.cpp
    src/ItemViews.cpp
    src/LineEdit.cpp
    src/Menu.cpp
    src/MenuBar.cpp
    src/MetricStyle.h
    src/Placement.h
    src/ProgressBar.cpp
    src/PushButton.cpp
    src/RadioButton.cpp
    src/ScrollBar.cpp
    src/Shadow.cpp
    src/SpinBox.cpp
    src/TabBar.cpp
    src/TeachingTip.cpp
    src/TextEdit.cpp
    src/Theme.cpp
    src/ToolButton.cpp
    src/ToolStyle.h
    src/ToolTip.cpp
    src/Util.h
    src/Version.cpp
    include/QLight/Calendar.h
    include/QLight/Card.h
    include/QLight/CheckBox.h
    include/QLight/ComboBox.h
    include/QLight/Dialog.h
    include/QLight/FluentIcon.h
    include/QLight/Flyout.h
    include/QLight/InfoBar.h
    include/QLight/ItemViews.h
    include/QLight/LineEdit.h
    include/QLight/Menu.h
    include/QLight/MenuBar.h
    include/QLight/ProgressBar.h
    include/QLight/PushButton.h
    include/QLight/RadioButton.h
    include/QLight/ScrollBar.h
    include/QLight/Shadow.h
    include/QLight/SpinBox.h
    include/QLight/TabBar.h
    include/QLight/TeachingTip.h
    include/QLight/TextEdit.h
    include/QLight/Theme.h
    include/QLight/ToolButton.h
    include/QLight/ToolTip.h
)
add_library(QLight::Widgets ALIAS qlight-widgets)

set_target_properties(qlight-widgets PROPERTIES
    AUTOMOC ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

include(GenerateExportHeader)
generate_export_header(qlight-widgets
    BASE_NAME QLIGHT
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/QLight/Export.h)
configure_file(cmake/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/QLight/Version.h @ONLY)

target_compile_definitions(qlight-widgets
    PUBLIC QLIGHT_STATIC_DEFINE
    PRIVATE QLIGHT_VERSION_STRING_LITERAL="${PROJECT_VERSION}")
target_include_directories(qlight-widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(qlight-widgets PUBLIC Qt6::Widgets PRIVATE Qt6::Svg)
if(MSVC)
    target_compile_options(qlight-widgets PRIVATE /utf-8)
endif()

# Built-in icons: Fluent UI System Icons subset (MIT), embedded as Qt resources.
file(GLOB QLIGHT_ICON_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/*.svg)
qt_add_resources(qlight-widgets qlight_icons
    PREFIX /qlight/icons
    BASE ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons
    FILES ${QLIGHT_ICON_FILES})

if(QLIGHT_WITH_WINDOWKIT)
    # QWindowKit (Apache-2.0) keeps native window behaviour (Aero Snap, resize
    # borders, system menu, Windows 11 snap layouts) under the custom title bar.
    include(FetchContent)
    set(QWINDOWKIT_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(QWINDOWKIT_BUILD_WIDGETS ON CACHE BOOL "" FORCE)
    set(QWINDOWKIT_BUILD_QUICK OFF CACHE BOOL "" FORCE)
    set(QWINDOWKIT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(QWINDOWKIT_BUILD_DOCUMENTATIONS OFF CACHE BOOL "" FORCE)
    set(QWINDOWKIT_ENABLE_STYLE_AGENT OFF CACHE BOOL "" FORCE)
    set(QWINDOWKIT_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(qwindowkit
        GIT_REPOSITORY https://github.com/stdware/qwindowkit.git
        GIT_TAG 1.5.0
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(qwindowkit)

    target_sources(qlight-widgets PRIVATE
        src/FramelessWindow.cpp
        include/QLight/FramelessWindow.h)
    target_link_libraries(qlight-widgets PRIVATE QWKWidgets)
    target_compile_definitions(qlight-widgets PUBLIC QLIGHT_HAS_FRAMELESS)
endif()
