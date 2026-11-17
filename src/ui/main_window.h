// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <QWidget>

#ifdef QLIGHT_HAS_FRAMELESS
#include <QLight/FramelessWindow.h>
#endif

class AppController;

namespace ui {

class MainView;

// Top-level window: a QLight::FramelessWindow (Fluent title bar on top of
// QWindowKit, which keeps native moving/resizing, Aero Snap and the system
// menu on Windows, and the native traffic lights on macOS). The plain-QWidget
// fallback is only used when QLight is built without QWindowKit.
#ifdef QLIGHT_HAS_FRAMELESS
using MainWindowBase = QLight::FramelessWindow;
#else
using MainWindowBase = QWidget;
#endif

class MainWindow : public MainWindowBase {
    Q_OBJECT
public:
    explicit MainWindow(AppController *controller, QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *e) override;

private:
    void retranslateUi();

    MainView *view_;
};

}  // namespace ui
