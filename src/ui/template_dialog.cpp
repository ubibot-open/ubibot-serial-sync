// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/template_dialog.h"

#include "app/app_controller.h"
#include "ui/ui_common.h"

#include <QVBoxLayout>

#include <QLight/LineEdit.h>
#include <QLight/PushButton.h>
#include <QLight/TextEdit.h>

namespace ui {

TemplateDialog::TemplateDialog(AppController *controller, QWidget *parent)
    : AppDialog(parent), controller_(controller) {
    setDialogWidth(380);
    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(6);
    nameLabel_ = new ThemedLabel(ThemedLabel::Primary, content);
    nameField_ = new QLight::LineEdit(content);
    contentLabel_ = new ThemedLabel(ThemedLabel::Primary, content);
    contentField_ = new QLight::PlainTextEdit(content);
    contentField_->setFont(monoFont());
    contentField_->setFixedHeight(96);
    col->addWidget(nameLabel_);
    col->addWidget(nameField_);
    col->addSpacing(6);
    col->addWidget(contentLabel_);
    col->addWidget(contentField_);
    setContentWidget(content);

    cancelButton_ = addRejectButton(QString());
    saveButton_ = addActionButton(QString(), true);
    connect(saveButton_, &QPushButton::clicked, this, &TemplateDialog::save);
    connect(nameField_, &QLineEdit::textChanged, this, &TemplateDialog::updateSaveEnabled);
    connect(contentField_, &QPlainTextEdit::textChanged, this, &TemplateDialog::updateSaveEnabled);
    retranslateUi();
}

void TemplateDialog::retranslateUi() {
    setTitle(editRow_ >= 0 ? tr("Edit template") : tr("New template"));
    nameLabel_->setText(tr("Name"));
    nameField_->setPlaceholderText(tr("e.g. Reset device"));
    contentLabel_->setText(tr("Content"));
    contentField_->setPlaceholderText(tr("The literal text to send"));
    cancelButton_->setText(tr("Cancel"));
    saveButton_->setText(tr("Save"));
}

void TemplateDialog::openForNew() {
    editRow_ = -1;
    nameField_->clear();
    contentField_->clear();
    retranslateUi();
    updateSaveEnabled();
    open();
    nameField_->setFocus();
}

void TemplateDialog::openForEdit(int row, const QString &name, const QString &content) {
    editRow_ = row;
    nameField_->setText(name);
    contentField_->setPlainText(content);
    retranslateUi();
    updateSaveEnabled();
    open();
    nameField_->setFocus();
}

void TemplateDialog::updateSaveEnabled() {
    saveButton_->setEnabled(!nameField_->text().trimmed().isEmpty() &&
                            !contentField_->toPlainText().trimmed().isEmpty());
}

void TemplateDialog::save() {
    if (!saveButton_->isEnabled()) return;
    if (editRow_ >= 0) controller_->updateCustomTemplate(editRow_, nameField_->text(), contentField_->toPlainText());
    else controller_->addCustomTemplate(nameField_->text(), contentField_->toPlainText());
    accept();
}

}  // namespace ui
