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

// Top-level window. On Windows it is a QLight::FramelessWindow (Fluent title
// bar on top of QWindowKit, which keeps Aero Snap, native resize borders and
// the system menu); on macOS and Linux it is a plain window with the native
// frame, where QWindowKit is untested.
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
