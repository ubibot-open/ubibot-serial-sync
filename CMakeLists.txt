cmake_minimum_required(VERSION 3.21)

project(UbiBotSerialAssistant
    VERSION 1.0.0
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
        qml/SerialSettingsPanel.qml
        qml/CommandLibraryPanel.qml
        qml/RemoteAssistPanel.qml
        qml/CommandParamsPanel.qml
        qml/DataMonitorView.qml
        qml/ConnectionWizardDialog.qml
        qml/SaveLogDialog.qml
        qml/SettingsAboutDialog.qml
    SOURCES
        src/core/language_manager.h
        src/core/language_manager.cpp
        src/core/device_library.h
        src/core/device_library.cpp
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
