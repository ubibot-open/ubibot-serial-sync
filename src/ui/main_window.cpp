// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/main_window.h"

#include "ui/main_view.h"

#include <QEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>

namespace ui {

MainWindow::MainWindow(AppController *controller, QWidget *parent) : MainWindowBase(parent) {
    view_ = new MainView(controller, this);
#ifdef QLIGHT_HAS_FRAMELESS
    setCentralWidget(view_);
#else
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view_);
#endif
    setWindowIcon(QIcon(QStringLiteral(":/icons/app.png")));
    setMinimumSize(860, 600);

    // 1320 x 860 (the design size), shrunk to fit smaller screens
    QSize size(1320, 860);
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QRect avail = screen->availableGeometry();
        size = size.boundedTo(avail.size() - QSize(40, 40));
        resize(size);
        move(avail.center() - QPoint(size.width() / 2, size.height() / 2));
    } else {
        resize(size);
    }
    retranslateUi();
}

void MainWindow::changeEvent(QEvent *e) {
    MainWindowBase::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) retranslateUi();
}

void MainWindow::retranslateUi() { setWindowTitle(tr("UbiBot Serial Assistant")); }

}  // namespace ui
