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
set(CMAKE_AUTOUIC ON)

# Qt 6.11 is what this project targets; anything >= 6.7 (for qt_add_translations)
# will work, but 6.11 is the minimum we've validated against.
# (Network isn't linked yet: remote support is a UI placeholder for now --
#  see README.md's "Project notes" -- add Qt6::Network back once a real
#  transport is wired up.)
find_package(Qt6 6.7 REQUIRED COMPONENTS Widgets SerialPort LinguistTools)

qt_standard_project_setup()

set(APP_SOURCES
    src/main.cpp

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

    src/ui/styles.h
    src/ui/styles.cpp
    src/ui/flow_layout.h
    src/ui/flow_layout.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.cpp
    src/ui/serial_settings_panel.h
    src/ui/serial_settings_panel.cpp
    src/ui/command_library_panel.h
    src/ui/command_library_panel.cpp
    src/ui/remote_assist_panel.h
    src/ui/remote_assist_panel.cpp
    src/ui/command_params_panel.h
    src/ui/command_params_panel.cpp
    src/ui/data_monitor_view.h
    src/ui/data_monitor_view.cpp
    src/ui/connection_wizard.h
    src/ui/connection_wizard.cpp
    src/ui/save_log_dialog.h
    src/ui/save_log_dialog.cpp
    src/ui/settings_about_dialog.h
    src/ui/settings_about_dialog.cpp

    resources/resources.qrc
)

qt_add_executable(UbiBotSerialAssistant
    WIN32 MACOSX_BUNDLE
    ${APP_SOURCES}
)

target_include_directories(UbiBotSerialAssistant PRIVATE
    src
)

target_link_libraries(UbiBotSerialAssistant PRIVATE
    Qt6::Widgets
    Qt6::SerialPort
)

# --- Translations -----------------------------------------------------
# Source strings (tr()) are written in English. ubibot_zh_CN.ts supplies the
# Simplified Chinese catalog; "Bilingual" mode is synthesized at runtime by
# combining the Chinese translation with the English source text (see
# core/language_manager.*), so it needs no separate .ts file. Add more
# ubibot_<locale>.ts files here as new languages are supported.
qt_add_translations(UbiBotSerialAssistant
    TS_FILES translations/ubibot_zh_CN.ts
    RESOURCE_PREFIX "/i18n"
    LUPDATE_OPTIONS -no-obsolete -locations none
)

set_target_properties(UbiBotSerialAssistant PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.ubibot.serialassistant
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)

install(TARGETS UbiBotSerialAssistant
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
