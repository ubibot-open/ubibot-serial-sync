// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "ui/ui_common.h"

#include <QVariantMap>

class AppController;
class QGridLayout;

namespace QLight {
class LineEdit;
class PushButton;
}  // namespace QLight

namespace ui {

// "Needs parameters" panel: one field per parameter of the clicked command,
// a live preview of the resolved command, and Insert (into the send box).
class CommandParamsPanel : public Panel {
    Q_OBJECT
public:
    explicit CommandParamsPanel(AppController *controller, QWidget *parent = nullptr);

    void openForRow(int row);

protected:
    void changeEvent(QEvent *e) override;

private:
    void retranslateUi();
    void clearFields();
    void updatePreview();

    AppController *controller_;
    int row_ = -1;
    QString commandName_;
    QVariantMap values_;

    ThemedLabel *title_;
    QGridLayout *fields_;
    ThemedLabel *previewLabel_;
    ThemedLabel *preview_;
    QLight::PushButton *cancelButton_;
    QLight::PushButton *insertButton_;
};

}  // namespace ui
