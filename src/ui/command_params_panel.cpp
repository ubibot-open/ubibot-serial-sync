#include "ui/command_params_panel.h"

#include <QEvent>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

CommandParamsPanel::CommandParamsPanel(QWidget *parent) : QWidget(parent) {
    buildUi();
    retranslateUi();
    setVisible(false);
}

void CommandParamsPanel::buildUi() {
    setStyleSheet(QStringLiteral("CommandParamsPanel { border: 1px solid #c9c9ca; }"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(11);

    auto *headerRow = new QHBoxLayout;
    nameLabel_ = new QLabel;
    QFont nameFont = nameLabel_->font();
    nameFont.setBold(true);
    nameFont.setPointSize(nameFont.pointSize() + 1);
    nameLabel_->setFont(nameFont);
    tagLabel_ = new QLabel;
    tagLabel_->setStyleSheet(
        QStringLiteral("border: 1px solid #5980a6; color: #5980a6; padding: 2px 8px; font-size: 11px;"));
    cancelButton_ = new QPushButton;
    headerRow->addWidget(nameLabel_);
    headerRow->addWidget(tagLabel_);
    headerRow->addStretch();
    headerRow->addWidget(cancelButton_);
    connect(cancelButton_, &QPushButton::clicked, this, &CommandParamsPanel::cancelled);

    fieldsLayout_ = new QGridLayout;
    fieldsLayout_->setHorizontalSpacing(10);
    fieldsLayout_->setVerticalSpacing(6);

    auto *footerRow = new QHBoxLayout;
    previewLabel_ = new QLabel;
    previewLabel_->setStyleSheet(QStringLiteral("font-family: Consolas, monospace; color: #416180; font-size: 12px;"));
    sendButton_ = new QPushButton;
    sendButton_->setProperty("primary", true);
    connect(sendButton_, &QPushButton::clicked, this,
            [this] { emit sendRequested(command_.resolve(values_)); });
    footerRow->addWidget(previewLabel_, 1);
    footerRow->addWidget(sendButton_);

    root->addLayout(headerRow);
    root->addLayout(fieldsLayout_);
    root->addLayout(footerRow);
}

void CommandParamsPanel::setCommand(const DeviceCommand &cmd) {
    command_ = cmd;
    values_.clear();

    QLayoutItem *item;
    while ((item = fieldsLayout_->takeAt(0))) {
        delete item->widget();
        delete item;
    }
    fieldEdits_.clear();

    nameLabel_->setText(cmd.name.text());

    int col = 0;
    for (const CommandParam &param : cmd.params) {
        values_[param.key] = param.defaultValue;

        auto *fieldBox = new QVBoxLayout;
        auto *label = new QLabel(param.label.text());
        label->setStyleSheet(QStringLiteral("font-size: 11px; color: #5d5d60;"));
        auto *edit = new QLineEdit(param.defaultValue);
        edit->setPlaceholderText(param.hint);
        edit->setStyleSheet(QStringLiteral("font-family: Consolas, monospace;"));
        const QString key = param.key;
        connect(edit, &QLineEdit::textChanged, this, [this, key](const QString &text) {
            values_[key] = text;
            updatePreview();
        });
        fieldEdits_.push_back(edit);

        auto *cell = new QWidget;
        auto *cellLayout = new QVBoxLayout(cell);
        cellLayout->setContentsMargins(0, 0, 0, 0);
        cellLayout->setSpacing(2);
        cellLayout->addWidget(label);
        cellLayout->addWidget(edit);
        fieldsLayout_->addWidget(cell, 0, col++);
    }

    setVisible(true);
    updatePreview();
}

void CommandParamsPanel::updatePreview() {
    previewLabel_->setText(command_.resolve(values_));
}

void CommandParamsPanel::retranslateUi() {
    tagLabel_->setText(tr("Needs parameters"));
    cancelButton_->setText(tr("Cancel"));
    sendButton_->setText(tr("Send"));
    if (!command_.name.zh.isEmpty()) nameLabel_->setText(command_.name.text());
}

void CommandParamsPanel::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
