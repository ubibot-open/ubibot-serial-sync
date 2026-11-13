# Copyright (C) 2026 UbiBot Co., Ltd.
# SPDX-License-Identifier: LGPL-3.0-only

cmake_minimum_required(VERSION 3.21)


project(UbiBotSerialAssistant
    VERSION 0.7.9.1
    DESCRIPTION "UbiBot Serial Assistant"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.7 REQUIRED COMPONENTS Quick QuickControls2 SerialPort Network LinguistTools)

qt_standard_project_setup(REQUIRES 6.7)

qt_add_executable(UbiBotSerialAssistant
    WIN32 MACOSX_BUNDLE
    src/main.cpp
)

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
    Qt6::Network
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE true)

qt_add_qml_module(UbiBotSerialAssistant
    URI UbiBot
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/DialogCard.qml
        qml/ModalDim.qml
        qml/PortComboBox.qml
        qml/SerialSettingsPanel.qml
        qml/CommandLibraryPanel.qml
        qml/RemoteAssistPanel.qml
        qml/CommandParamsPanel.qml
        qml/DataMonitorView.qml
        qml/SaveLogDialog.qml
        qml/SettingsAboutDialog.qml
        qml/AboutDialog.qml
        qml/SoftwareUpdateDialog.qml
        qml/DriverHintDialog.qml
        qml/DriverListDialog.qml
        qml/ComboBox.qml
        qml/TextField.qml
        qml/TextArea.qml
        qml/CheckBox.qml
        qml/RadioButton.qml
        qml/SpinBox.qml
        qml/Menu.qml
    SOURCES
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
