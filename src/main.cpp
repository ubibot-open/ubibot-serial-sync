// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "app/app_controller.h"
#include "core/settings_store.h"
#include "ui/main_window.h"
#include "ui/ui_common.h"

#include <QApplication>
#include <QIcon>

#include <QLight/ToolTip.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setOrganizationName(QStringLiteral("UbiBot"));
    QApplication::setOrganizationDomain(QStringLiteral("ubibot.com"));
    QApplication::setApplicationName(QStringLiteral("UbiBotSerialAssistant"));
    // APP_VERSION comes from CMakeLists.txt's `project(VERSION ...)` -- one
    // place to bump per release.
    QApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/app.png")));

    // Apply the persisted "System font" setting (Settings & About) before any
    // widget exists, so it is the default from the very first frame. See
    // AppController::buildApplicationFont() for the CJK fallback handling.
    {
        SettingsStore settings;
        QApplication::setFont(
            AppController::buildApplicationFont(settings.systemFontFamily(), settings.systemFontSize()));
    }

    // Fusion as the base style: QLight paints its own controls, and Fusion
    // keeps the few stock widgets left (scroll areas, file dialogs on Linux)
    // consistent across platforms.
    QApplication::setStyle(QStringLiteral("Fusion"));

    AppController controller;

    ui::applyTheme(controller.themeMode());
    QObject::connect(&controller, &AppController::themeModeChanged, &app,
                     [&controller] { ui::applyTheme(controller.themeMode()); });
    QLight::ToolTip::installGlobally();

    ui::MainWindow window(&controller);
    window.show();
    return app.exec();
}
