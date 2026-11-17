// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "ui/app_dialog.h"

#include <QVariantList>

class AppController;
class QScrollArea;
class QVBoxLayout;

namespace QLight {
class LineEdit;
class PushButton;
class SpinBox;
}  // namespace QLight

namespace ui {

class BatchEditDialog;
class ThemedLabel;

// List of saved batch commands, each with Run / Stop; right-click to edit or delete.
class BatchListDialog : public AppDialog {
    Q_OBJECT
public:
    explicit BatchListDialog(AppController *controller, QWidget *parent = nullptr);

protected:
    void retranslateUi() override;

private:
    void rebuildRows();
    void showRowMenu(int row, const QPoint &globalPos);

    AppController *controller_;
    QWidget *rowsHost_;
    QVBoxLayout *rowsLayout_;
    QScrollArea *scroll_;
    ThemedLabel *emptyLabel_;
    QLight::PushButton *newButton_;
    QLight::PushButton *closeButton_;
    BatchEditDialog *editDialog_ = nullptr;
};

// New / edit one batch command: name, interval and the list of steps.
class BatchEditDialog : public AppDialog {
    Q_OBJECT
public:
    explicit BatchEditDialog(AppController *controller, QWidget *parent = nullptr);

    void openForNew();
    void openForEdit(int row, const QString &name, int intervalMs);

protected:
    void retranslateUi() override;

private:
    struct StepRow;

    void clearSteps();
    void addStep(const QString &text = {}, bool isHex = false, bool crc = false);
    void removeStep(QWidget *rowWidget);
    QVariantList collectSteps() const;
    void updateSaveEnabled();
    void save();

    AppController *controller_;
    int editRow_ = -1;
    ThemedLabel *nameLabel_;
    QLight::LineEdit *nameField_;
    ThemedLabel *intervalLabel_;
    QLight::SpinBox *intervalSpin_;
    ThemedLabel *msLabel_;
    ThemedLabel *stepsLabel_;
    QWidget *stepsHost_;
    QVBoxLayout *stepsLayout_;
    QLight::PushButton *addStepButton_;
    QLight::PushButton *cancelButton_;
    QLight::PushButton *saveButton_;
};

}  // namespace ui
