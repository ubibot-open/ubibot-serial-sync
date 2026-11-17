// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <QLight/Dialog.h>

namespace QLight {
class PushButton;
}

namespace ui {

// QLight content dialog (rounded card over the dimmed main window, command
// strip at the bottom) plus what the app's dialogs need on top of it:
// footer buttons that don't close the dialog, re-fitting the height when the
// content changes while open, and live retranslation.
class AppDialog : public QLight::Dialog {
    Q_OBJECT
public:
    explicit AppDialog(QWidget *parent = nullptr);

protected:
    // Footer button that only emits clicked() (the dialog stays open).
    QLight::PushButton *addActionButton(const QString &text, bool primary = false);

    // Re-computes the height from the content and keeps the card centred.
    void refit();

    // Called once from the subclass constructor and on every language change.
    virtual void retranslateUi() {}

    void changeEvent(QEvent *e) override;
};

}  // namespace ui
