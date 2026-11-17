// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "ui/app_dialog.h"

class AppController;

namespace QLight {
class LineEdit;
class PlainTextEdit;
class PushButton;
}  // namespace QLight

namespace ui {

class ThemedLabel;

// New / edit a "My templates" quick-send entry.
class TemplateDialog : public AppDialog {
    Q_OBJECT
public:
    explicit TemplateDialog(AppController *controller, QWidget *parent = nullptr);

    void openForNew();
    void openForEdit(int row, const QString &name, const QString &content);

protected:
    void retranslateUi() override;

private:
    void updateSaveEnabled();
    void save();

    AppController *controller_;
    int editRow_ = -1;
    ThemedLabel *nameLabel_;
    ThemedLabel *contentLabel_;
    QLight::LineEdit *nameField_;
    QLight::PlainTextEdit *contentField_;
    QLight::PushButton *cancelButton_;
    QLight::PushButton *saveButton_;
};

}  // namespace ui
