// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/app_dialog.h"

#include <QEvent>
#include <QLayout>

#include <QLight/PushButton.h>

namespace ui {

AppDialog::AppDialog(QWidget *parent) : QLight::Dialog(parent) {}

QLight::PushButton *AppDialog::addActionButton(const QString &text, bool primary) {
    QLight::PushButton *b = addButton(text, QDialog::Rejected, primary);
    // drop the "close with result" connection Dialog::addButton made
    disconnect(b, &QPushButton::clicked, this, nullptr);
    b->setAutoDefault(false);
    b->setDefault(false);
    return b;
}

void AppDialog::refit() {
    if (!isVisible()) return;
    layout()->activate();
    const int h = layout()->totalHeightForWidth(width());
    const int newHeight = h > 0 ? h : sizeHint().height();
    if (newHeight == height()) return;
    const QPoint center = geometry().center();
    setFixedSize(width(), newHeight);
    move(center - QPoint(width() / 2, newHeight / 2));
}

void AppDialog::changeEvent(QEvent *e) {
    QLight::Dialog::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        retranslateUi();
        refit();
    }
}

}  // namespace ui
