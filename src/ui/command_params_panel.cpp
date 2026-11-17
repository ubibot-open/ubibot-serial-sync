// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/command_params_panel.h"

#include "app/app_controller.h"

#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QLight/LineEdit.h>
#include <QLight/PushButton.h>

namespace ui {

CommandParamsPanel::CommandParamsPanel(AppController *controller, QWidget *parent)
    : Panel(parent), controller_(controller) {
    setRecessed(true);
    hide();

    auto *outer = new QHBoxLayout(this);
    outer->setContentsMargins(16, 12, 12, 12);
    outer->setSpacing(16);

    auto *col = new QVBoxLayout;
    col->setSpacing(8);
    title_ = new ThemedLabel(ThemedLabel::Primary, this);
    title_->setEmphasis(true);
    col->addWidget(title_);
    fields_ = new QGridLayout;
    fields_->setHorizontalSpacing(16);
    fields_->setVerticalSpacing(6);
    col->addLayout(fields_);
    auto *previewRow = new QHBoxLayout;
    previewRow->setSpacing(16);
    previewLabel_ = new ThemedLabel(ThemedLabel::Secondary, this);
    preview_ = new ThemedLabel(ThemedLabel::Accent, this);
    preview_->setFont(monoFont());
    preview_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    preview_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    previewRow->addWidget(previewLabel_);
    previewRow->addWidget(preview_, 1);
    col->addLayout(previewRow);
    outer->addLayout(col, 1);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(8);
    cancelButton_ = new QLight::PushButton(this);
    insertButton_ = new QLight::PushButton(QString(), QLight::PushButton::Accent, this);
    cancelButton_->setMinimumWidth(100);
    insertButton_->setMinimumWidth(100);
    buttons->addWidget(cancelButton_);
    buttons->addWidget(insertButton_);
    outer->addLayout(buttons);
    outer->setAlignment(buttons, Qt::AlignBottom);

    connect(cancelButton_, &QPushButton::clicked, this, &QWidget::hide);
    connect(insertButton_, &QPushButton::clicked, this, [this] {
        controller_->loadCommandWithParamsIntoDraft(row_, values_);
        hide();
    });
    // the row index is only valid for the command list it came from
    connect(controller_->commandModel(), &QAbstractItemModel::modelReset, this, &QWidget::hide);
    connect(controller_->commandModel(), &QAbstractItemModel::rowsMoved, this, &QWidget::hide);
    connect(controller_->commandModel(), &QAbstractItemModel::rowsRemoved, this, &QWidget::hide);
    retranslateUi();
}

void CommandParamsPanel::changeEvent(QEvent *e) {
    Panel::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) retranslateUi();
}

void CommandParamsPanel::retranslateUi() {
    title_->setText(commandName_.isEmpty() ? tr("Needs parameters")
                                           : tr("Needs parameters") + QStringLiteral("  ·  ") + commandName_);
    previewLabel_->setText(tr("Preview"));
    cancelButton_->setText(tr("Cancel"));
    insertButton_->setText(tr("Insert"));
}

void CommandParamsPanel::clearFields() {
    while (QLayoutItem *item = fields_->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
}

void CommandParamsPanel::openForRow(int row) {
    row_ = row;
    commandName_ = controller_->commandNameForRow(row);
    values_.clear();
    clearFields();

    const QVariantList params = controller_->paramsForRow(row);
    QLight::LineEdit *first = nullptr;
    for (int i = 0; i < params.size(); ++i) {
        const QVariantMap p = params.at(i).toMap();
        const QString key = p.value(QStringLiteral("key")).toString();
        const QString value = p.value(QStringLiteral("defaultValue")).toString();
        values_.insert(key, value);

        auto *label = new ThemedLabel(p.value(QStringLiteral("label")).toString(), ThemedLabel::Primary, this);
        auto *field = new QLight::LineEdit(value, this);
        field->setFont(monoFont());
        field->setPlaceholderText(p.value(QStringLiteral("hint")).toString());
        field->setMinimumWidth(160);
        connect(field, &QLineEdit::textEdited, this, [this, key](const QString &text) {
            values_.insert(key, text);
            updatePreview();
        });
        // two parameter pairs per row
        const int r = i / 2;
        const int c = (i % 2) * 2;
        fields_->addWidget(label, r, c);
        fields_->addWidget(field, r, c + 1);
        if (!first) first = field;
    }
    fields_->setColumnStretch(1, 1);
    fields_->setColumnStretch(3, params.size() > 1 ? 1 : 0);

    retranslateUi();
    updatePreview();
    show();
    if (first) {
        first->setFocus();
        first->selectAll();
    }
}

void CommandParamsPanel::updatePreview() {
    preview_->setText(row_ >= 0 ? controller_->previewCommand(row_, values_) : QString());
}

}  // namespace ui
