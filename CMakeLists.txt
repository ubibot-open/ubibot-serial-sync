# Copyright (C) 2026 UbiBot Co., Ltd.
# SPDX-License-Identifier: LGPL-3.0-only

cmake_minimum_required(VERSION 3.21)


project(UbiBotSerialAssistant
    VERSION 0.9.1.2
    DESCRIPTION "UbiBot Serial Assistant"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.7 REQUIRED COMPONENTS Widgets Svg SerialPort Network LinguistTools)

qt_standard_project_setup(REQUIRES 6.7)

# Fluent-style widget library (vendored, MIT) -- see third_party/qlight/CMakeLists.txt.
add_subdirectory(third_party/qlight)
# QWindowKit (pulled in by QLight for the title bar) links Qt's private modules; since
# Qt 6.9 those are separate packages that must also be visible at this level.
# QUIET: older Qt versions don't have them as separate packages at all.
set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6 QUIET COMPONENTS CorePrivate GuiPrivate WidgetsPrivate)

qt_add_executable(UbiBotSerialAssistant
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/core/language_manager.h
    src/core/language_manager.cpp
    src/core/device_library.h
    src/core/device_library.cpp
    src/core/device_library_update_client.h
    src/core/device_library_update_client.cpp
    src/core/software_update_client.h
    src/core/software_update_client.cpp
    src/core/self_update_installer.h
    src/core/self_update_installer.cpp
    src/core/driver_check.h
    src/core/driver_check.cpp
    src/core/env_config.h
    src/core/env_config.cpp
    src/core/ansi_text.h
    src/core/ansi_text.cpp
    src/core/hex_text.h
    src/core/hex_text.cpp
    src/core/crc16.h
    src/core/crc16.cpp
    src/core/batch_command.h
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
    src/models/batch_command_model.h
    src/models/batch_command_model.cpp
    src/models/port_list_model.h
    src/models/port_list_model.cpp
    src/app/app_controller.h
    src/app/app_controller.cpp
    src/app/serial_options.h
    src/app/serial_options.cpp
    src/ui/ui_common.h
    src/ui/ui_common.cpp
    src/ui/app_dialog.h
    src/ui/app_dialog.cpp
    src/ui/data_monitor_view.h
    src/ui/data_monitor_view.cpp
    src/ui/serial_settings_panel.h
    src/ui/serial_settings_panel.cpp
    src/ui/command_library_panel.h
    src/ui/command_library_panel.cpp
    src/ui/command_params_panel.h
    src/ui/command_params_panel.cpp
    src/ui/template_dialog.h
    src/ui/template_dialog.cpp
    src/ui/batch_dialogs.h
    src/ui/batch_dialogs.cpp
    src/ui/misc_dialogs.h
    src/ui/misc_dialogs.cpp
    src/ui/settings_dialog.h
    src/ui/settings_dialog.cpp
    src/ui/main_view.h
    src/ui/main_view.cpp
    src/ui/main_window.h
    src/ui/main_window.cpp
)

target_include_directories(UbiBotSerialAssistant PRIVATE
    src
    src/core
    src/models
    src/app
)

target_link_libraries(UbiBotSerialAssistant PRIVATE
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Network
    QLight::Widgets
)

# No .ui files; AUTOUIC would otherwise mistake the "ui/ui_*.h" headers for
# generated uic output.
set_target_properties(UbiBotSerialAssistant PROPERTIES AUTOUIC OFF)

if(MSVC)
    target_compile_options(UbiBotSerialAssistant PRIVATE /utf-8)
endif()

target_sources(UbiBotSerialAssistant PRIVATE
    resources/resources.qrc
)

set_target_properties(UbiBotSerialAssistant PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.ubibot.serialassistant
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)


string(TIMESTAMP APP_BUILD_TIME "%Y-%m-%d %H:%M UTC" UTC)
target_compile_definitions(UbiBotSerialAssistant PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
    APP_BUILD_TIME="${APP_BUILD_TIME}"
    APP_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

if(WIN32)
    configure_file(
        resources/app_version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/app_version.rc
        @ONLY
    )
    target_sources(UbiBotSerialAssistant PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/app_version.rc
    )

    # Declare Windows 10 as the minimum in the PE header (the default is 6.0,
    # i.e. Vista). Qt 6 needs Windows 10 anyway, and an honest minimum makes
    # the loader show "This application requires a newer version of Windows"
    # on Windows 7/8.1 instead of the misleading "icu.dll was not found" that
    # falls out of Qt's own ICU import (icuuc.dll forwards to the OS's
    # icu.dll, which only exists since Windows 10 1703).
    if(MSVC)
        target_link_options(UbiBotSerialAssistant PRIVATE /SUBSYSTEM:WINDOWS,10.0)
    else()
        target_link_options(UbiBotSerialAssistant PRIVATE -Wl,--subsystem,windows:10.0)
    endif()
endif()

qt_add_translations(UbiBotSerialAssistant
    TS_FILES
        translations/ubibot_zh_CN.ts
        translations/ubibot_zh_TW.ts
        translations/ubibot_ja.ts
        translations/ubibot_ko.ts
        translations/ubibot_ru.ts
        translations/ubibot_fr.ts
        translations/ubibot_it.ts
    RESOURCE_PREFIX "/i18n"
    LUPDATE_OPTIONS -no-obsolete -locations none
)

install(TARGETS UbiBotSerialAssistant
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
