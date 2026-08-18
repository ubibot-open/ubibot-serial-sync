cmake_minimum_required(VERSION 3.21)

# *** Bump this on every release -- it's the ONLY place the version number
# needs to change. CMake's PROJECT_VERSION (used below to feed main.cpp/
# AppController, the app's own display, and the macOS bundle info) and the
# Windows .exe's own file-version resource both come from this single
# MAJOR.MINOR.PATCH.TWEAK value. Four components (not the usual three) since
# that's what was asked for -- CMake's project(VERSION ...) already supports
# up to four.
project(UbiBotSerialAssistant
    VERSION 0.2.1.1
    DESCRIPTION "UbiBot Serial Assistant / UbiBot 串口助手"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt 6.11 is what this project targets; anything >= 6.7 (for
# qt_add_translations()) will work, but 6.11 is the minimum we've validated
# against. QtQuick.Dialogs' FolderDialog (SaveLogDialog.qml) needs 6.3+,
# well under our floor.
find_package(Qt6 6.7 REQUIRED COMPONENTS Quick QuickControls2 SerialPort LinguistTools)

qt_standard_project_setup(REQUIRES 6.7)

qt_add_executable(UbiBotSerialAssistant
    WIN32 MACOSX_BUNDLE
    src/main.cpp
)

# qmltyperegistrar's generated *_qmltyperegistrations.cpp #includes each
# QML_ELEMENT/QML_SINGLETON type's header by its bare file name (e.g.
# "app_controller.h"), not the "app/app_controller.h"-style relative path
# used elsewhere in this project's own #includes -- so every subdirectory
# that declares such a type needs to be on the include path too, or that
# generated file fails to compile with "'AppController' was not declared in
# this scope" (and likewise for every other registered type).
target_include_directories(UbiBotSerialAssistant PRIVATE
    src
    src/core
    src/models
    src/app
)

target_link_libraries(UbiBotSerialAssistant PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::SerialPort
)

# qt_add_qml_module() does NOT scan .qml file contents for "pragma
# Singleton" -- it only honors the QT_QML_SINGLETON_TYPE source property,
# which nothing sets by default. Without this, Theme.qml's pragma is
# silently ignored: the generated qmldir lists "Theme 1.0 Theme.qml" as a
# plain, instantiable component instead of "singleton Theme 1.0 Theme.qml",
# so every Theme.xxx reference across the app resolves to undefined with no
# warning printed (this is a WIN32_EXECUTABLE GUI app, so qWarning goes to
# OutputDebugString, not a visible console) -- the whole UI silently loses
# its color palette.
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE true)

# --- QML module --------------------------------------------------------
# Everything but main.cpp -- the backend (core/), the QML-facing adapter
# layer (models/, app/), and the .qml files themselves -- is registered as
# one QML module so QML_ELEMENT/QML_SINGLETON types in the C++ sources get
# picked up by qmltyperegistrar automatically (no manual qmlRegisterType
# calls anywhere).
qt_add_qml_module(UbiBotSerialAssistant
    URI UbiBot
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/DialogCard.qml
        qml/PortComboBox.qml
        qml/SerialSettingsPanel.qml
        qml/CommandLibraryPanel.qml
        qml/RemoteAssistPanel.qml
        qml/CommandParamsPanel.qml
        qml/DataMonitorView.qml
        qml/ConnectionWizardDialog.qml
        qml/SaveLogDialog.qml
        qml/SettingsAboutDialog.qml
        qml/AboutDialog.qml
    SOURCES
        src/core/language_manager.h
        src/core/language_manager.cpp
        src/core/device_library.h
        src/core/device_library.cpp
        src/core/ansi_text.h
        src/core/ansi_text.cpp
        src/core/crc16.h
        src/core/crc16.cpp
        src/core/log_entry.h
        src/core/log_manager.h
        src/core/log_manager.cpp
        src/core/serial_manager.h
        src/core/serial_manager.cpp
        src/core/settings_store.h
        src/core/settings_store.cpp
        src/models/log_list_model.h
        src/models/log_list_model.cpp
        src/models/command_list_model.h
        src/models/command_list_model.cpp
        src/models/command_history_model.h
        src/models/command_history_model.cpp
        src/models/port_list_model.h
        src/models/port_list_model.cpp
        src/app/app_controller.h
        src/app/app_controller.cpp
        src/app/serial_options.h
        src/app/serial_options.cpp
    RESOURCE_PREFIX "/qt/qml"
)

target_sources(UbiBotSerialAssistant PRIVATE
    resources/resources.qrc
)

set_target_properties(UbiBotSerialAssistant PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.ubibot.serialassistant
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)

# Every place the app needs its own version number (main.cpp's
# setApplicationVersion(), AppController::appVersion() for the title bar/
# Settings & About dialog, ...) reads it back from PROJECT_VERSION via this
# one compile definition, rather than each hardcoding its own copy of the
# string -- so bumping the version for a release really is just the one
# edit at the top of this file.
#
# APP_BUILD_TIME is stamped once here, at CMake *configure* time -- not
# reevaluated on every incremental compile -- and shown next to the version
# number in the About dialog (AppController::buildTime()). Good enough for
# "when was this build configured/produced"; a fresh `cmake -S . -B ...`
# (as the packaging steps below always start with) picks up a current
# timestamp for each real release.
string(TIMESTAMP APP_BUILD_TIME "%Y-%m-%d %H:%M UTC" UTC)
target_compile_definitions(UbiBotSerialAssistant PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
    APP_BUILD_TIME="${APP_BUILD_TIME}"
)

if(WIN32)
    # Stamps the built .exe with a real Windows file-version resource (see
    # app_version.rc.in) from the same PROJECT_VERSION -- otherwise a
    # Windows build has no version info in the binary at all, unlike the
    # macOS bundle above which already gets one.
    configure_file(
        resources/app_version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/app_version.rc
        @ONLY
    )
    target_sources(UbiBotSerialAssistant PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/app_version.rc
    )
endif()

# --- Translations -----------------------------------------------------
# Source strings (tr()/qsTr()) are written in English, which needs no .ts
# file of its own. ubibot_zh_CN.ts supplies the Simplified Chinese catalog.
# lupdate scans both the C++ sources above and the .qml files listed in
# QML_FILES natively. The language picker (Settings & About dialog) only
# ever shows one active language at a time, populated from
# AppController::availableLanguages() (core/language_manager.cpp).
#
# To add another language: list it in one more TS_FILES entry below, add a
# matching {code, nativeName} to kLanguages in core/language_manager.cpp,
# and fill in the .ts once lupdate has generated its <message> entries.
qt_add_translations(UbiBotSerialAssistant
    TS_FILES translations/ubibot_zh_CN.ts
    RESOURCE_PREFIX "/i18n"
    LUPDATE_OPTIONS -no-obsolete -locations none
)

install(TARGETS UbiBotSerialAssistant
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
