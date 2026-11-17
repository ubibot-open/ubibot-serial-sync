// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/batch_dialogs.h"

#include "app/app_controller.h"
#include "core/hex_text.h"
#include "ui/ui_common.h"

#include <QContextMenuEvent>
#include <QToolTip>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>

#include <QLight/CheckBox.h>
#include <QLight/FluentIcon.h>
#include <QLight/LineEdit.h>
#include <QLight/Menu.h>
#include <QLight/PushButton.h>
#include <QLight/ScrollBar.h>
#include <QLight/SpinBox.h>
#include <QLight/Theme.h>
#include <QLight/ToolButton.h>

namespace ui {

namespace {

// One saved batch in the list: highlighted while it is running.
class BatchRow : public QWidget {
public:
    BatchRow(bool running, QWidget *parent) : QWidget(parent), running_(running) {
        setContextMenuPolicy(Qt::DefaultContextMenu);
        setMinimumHeight(56);
    }
    std::function<void(const QPoint &)> onContextMenu;

protected:
    void paintEvent(QPaintEvent *) override {
        if (!running_) return;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(colors().accentTint);
        p.drawRoundedRect(QRectF(rect()), 6, 6);
    }
    void contextMenuEvent(QContextMenuEvent *e) override {
        if (onContextMenu) onContextMenu(e->globalPos());
    }

private:
    bool running_;
};

QScrollArea *makeScrollArea(QWidget *content, QWidget *parent) {
    auto *scroll = new QScrollArea(parent);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setWidget(content);
    content->setAutoFillBackground(false);  // setWidget() turns it on
    QLight::ScrollBar::overlayIn(scroll);
    return scroll;
}

}  // namespace

// --- BatchListDialog -----------------------------------------------------

BatchListDialog::BatchListDialog(AppController *controller, QWidget *parent)
    : AppDialog(parent), controller_(controller) {
    setDialogWidth(440);

    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(0);
    emptyLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    emptyLabel_->setWordWrap(true);
    col->addWidget(emptyLabel_);

    rowsHost_ = new QWidget;
    rowsLayout_ = new QVBoxLayout(rowsHost_);
    rowsLayout_->setContentsMargins(0, 0, 8, 0);
    rowsLayout_->setSpacing(0);
    scroll_ = makeScrollArea(rowsHost_, content);
    scroll_->setFixedHeight(280);
    col->addWidget(scroll_);
    setContentWidget(content);

    newButton_ = addActionButton(QString(), true);
    closeButton_ = addRejectButton(QString());
    connect(newButton_, &QPushButton::clicked, this, [this] {
        if (!editDialog_) editDialog_ = new BatchEditDialog(controller_, parentWidget());
        editDialog_->openForNew();
    });

    BatchCommandModel *model = controller_->batchCommandModel();
    connect(model, &QAbstractItemModel::modelReset, this, &BatchListDialog::rebuildRows);
    connect(model, &QAbstractItemModel::rowsInserted, this, &BatchListDialog::rebuildRows);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &BatchListDialog::rebuildRows);
    connect(model, &QAbstractItemModel::dataChanged, this, &BatchListDialog::rebuildRows);
    connect(controller_, &AppController::batchStateChanged, this, &BatchListDialog::rebuildRows);
    retranslateUi();
}

void BatchListDialog::retranslateUi() {
    setTitle(tr("Batch commands"));
    emptyLabel_->setText(tr("No batch commands yet"));
    newButton_->setText(tr("New batch"));
    closeButton_->setText(tr("Close"));
    rebuildRows();
}

void BatchListDialog::rebuildRows() {
    while (QLayoutItem *item = rowsLayout_->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
    BatchCommandModel *model = controller_->batchCommandModel();
    const int count = model->rowCount();
    emptyLabel_->setVisible(count == 0);
    scroll_->setVisible(count > 0);

    for (int row = 0; row < count; ++row) {
        const QModelIndex idx = model->index(row);
        const bool running = controller_->runningBatchRow() == row;
        auto *rowWidget = new BatchRow(running, rowsHost_);
        auto *h = new QHBoxLayout(rowWidget);
        h->setContentsMargins(12, 8, 12, 8);
        h->setSpacing(12);
        auto *texts = new QVBoxLayout;
        texts->setSpacing(2);
        auto *name = new ThemedLabel(idx.data(BatchCommandModel::NameRole).toString(), ThemedLabel::Primary, rowWidget);
        name->setEmphasis(true);
        const QString detail =
            running ? controller_->batchProgressText()
                    : tr("%1 steps · %2 ms interval")
                          .arg(idx.data(BatchCommandModel::StepCountRole).toInt())
                          .arg(idx.data(BatchCommandModel::IntervalMsRole).toInt());
        auto *sub = new ThemedLabel(detail, running ? ThemedLabel::Accent : ThemedLabel::Secondary, rowWidget);
        sub->setEmphasis(false, -1);
        texts->addWidget(name);
        texts->addWidget(sub);
        h->addLayout(texts, 1);

        auto *action = new QLight::PushButton(running ? tr("Stop") : tr("Run"), rowWidget);
        action->setFluentIcon(running ? QLight::Icon::Stop : QLight::Icon::Play);
        action->setMinimumWidth(88);
        connect(action, &QPushButton::clicked, this, [this, row, running] {
            if (running) controller_->stopBatchCommand();
            else controller_->startBatchCommand(row);
        });
        h->addWidget(action);
        rowWidget->onContextMenu = [this, row](const QPoint &pos) { showRowMenu(row, pos); };

        rowsLayout_->addWidget(rowWidget);
        if (row < count - 1) rowsLayout_->addWidget(new Divider(Qt::Horizontal, rowsHost_));
    }
    rowsLayout_->addStretch(1);
}

void BatchListDialog::showRowMenu(int row, const QPoint &globalPos) {
    QLight::Menu menu(this);
    QAction *edit = menu.addAction(QLight::FluentIcon(QLight::Icon::Edit).icon(), tr("Edit"));
    QAction *remove = menu.addAction(QLight::FluentIcon(QLight::Icon::Delete).icon(), tr("Delete"));
    QAction *chosen = menu.exec(globalPos);
    BatchCommandModel *model = controller_->batchCommandModel();
    const QModelIndex idx = model->index(row);
    if (!idx.isValid()) return;
    if (chosen == edit) {
        if (!editDialog_) editDialog_ = new BatchEditDialog(controller_, parentWidget());
        editDialog_->openForEdit(row, idx.data(BatchCommandModel::NameRole).toString(),
                                 idx.data(BatchCommandModel::IntervalMsRole).toInt());
    } else if (chosen == remove) {
        controller_->removeBatchCommand(row);
    }
}

// --- BatchEditDialog -----------------------------------------------------

BatchEditDialog::BatchEditDialog(AppController *controller, QWidget *parent)
    : AppDialog(parent), controller_(controller) {
    setDialogWidth(480);

    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(6);

    nameLabel_ = new ThemedLabel(ThemedLabel::Primary, content);
    nameField_ = new QLight::LineEdit(content);
    col->addWidget(nameLabel_);
    col->addWidget(nameField_);

    col->addSpacing(6);
    intervalLabel_ = new ThemedLabel(ThemedLabel::Primary, content);
    col->addWidget(intervalLabel_);
    auto *intervalRow = new QHBoxLayout;
    intervalSpin_ = new QLight::SpinBox(content);
    intervalSpin_->setRange(0, 3600000);
    intervalSpin_->setSingleStep(50);
    intervalSpin_->setMinimumWidth(120);
    msLabel_ = new ThemedLabel(QStringLiteral("ms"), ThemedLabel::Secondary, content);
    intervalRow->addWidget(intervalSpin_);
    intervalRow->addWidget(msLabel_);
    intervalRow->addStretch(1);
    col->addLayout(intervalRow);

    col->addSpacing(6);
    stepsLabel_ = new ThemedLabel(ThemedLabel::Primary, content);
    col->addWidget(stepsLabel_);
    stepsHost_ = new QWidget;
    stepsLayout_ = new QVBoxLayout(stepsHost_);
    stepsLayout_->setContentsMargins(0, 0, 8, 0);
    stepsLayout_->setSpacing(8);
    stepsLayout_->addStretch(1);
    auto *scroll = makeScrollArea(stepsHost_, content);
    scroll->setFixedHeight(200);
    col->addWidget(scroll);

    addStepButton_ = new QLight::PushButton(content);
    addStepButton_->setFluentIcon(QLight::Icon::Add);
    col->addWidget(addStepButton_, 0, Qt::AlignLeft);
    setContentWidget(content);

    cancelButton_ = addRejectButton(QString());
    saveButton_ = addActionButton(QString(), true);
    connect(saveButton_, &QPushButton::clicked, this, &BatchEditDialog::save);
    connect(addStepButton_, &QPushButton::clicked, this, [this] {
        addStep();
        updateSaveEnabled();
    });
    connect(nameField_, &QLineEdit::textChanged, this, &BatchEditDialog::updateSaveEnabled);
    retranslateUi();
}

void BatchEditDialog::retranslateUi() {
    setTitle(editRow_ >= 0 ? tr("Edit batch command") : tr("New batch command"));
    nameLabel_->setText(tr("Name"));
    nameField_->setPlaceholderText(tr("e.g. Startup sequence"));
    intervalLabel_->setText(tr("Interval"));
    msLabel_->setText(tr("ms"));
    stepsLabel_->setText(tr("Steps"));
    addStepButton_->setText(tr("Add step"));
    cancelButton_->setText(tr("Cancel"));
    saveButton_->setText(tr("Save"));
    for (int i = 0; i < stepsLayout_->count(); ++i) {
        QWidget *row = stepsLayout_->itemAt(i)->widget();
        if (!row) continue;
        if (auto *text = row->findChild<QLight::LineEdit *>()) text->setPlaceholderText(tr("Command text"));
        if (auto *crc = row->findChild<QLight::CheckBox *>(QStringLiteral("crc"))) crc->setText(tr("CRC"));
        if (auto *del = row->findChild<QLight::ToolButton *>()) del->setToolTip(tr("Remove step"));
    }
}

void BatchEditDialog::clearSteps() {
    while (stepsLayout_->count() > 1) {  // keep the trailing stretch
        QLayoutItem *item = stepsLayout_->takeAt(0);
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
}

void BatchEditDialog::addStep(const QString &text, bool isHex, bool crc) {
    auto *row = new QWidget(stepsHost_);
    auto *h = new QHBoxLayout(row);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    auto *field = new QLight::LineEdit(text, row);
    field->setFont(monoFont());
    field->setPlaceholderText(tr("Command text"));
    auto *hex = new QLight::CheckBox(QStringLiteral("HEX"), row);
    hex->setObjectName(QStringLiteral("hex"));
    hex->setChecked(isHex);
    auto *crcBox = new QLight::CheckBox(tr("CRC"), row);
    crcBox->setObjectName(QStringLiteral("crc"));
    crcBox->setChecked(crc);
    auto *del = new QLight::ToolButton(QLight::FluentIcon(QLight::Icon::Dismiss), row);
    del->setToolTip(tr("Remove step"));
    h->addWidget(field, 1);
    h->addWidget(hex);
    h->addWidget(crcBox);
    h->addWidget(del);
    connect(del, &QToolButton::clicked, this, [this, row] { removeStep(row); });
    connect(field, &QLineEdit::textChanged, this, &BatchEditDialog::updateSaveEnabled);
    // Same conversion as the main send box (see core/hex_text.h). clicked(), not
    // toggled(): only a user click converts, not the setChecked() above.
    connect(hex, &QCheckBox::clicked, this, [this, field](bool toHex) {
        if (field->text().trimmed().isEmpty()) return;
        if (toHex) {
            field->setText(HexText::fromAsciiDraft(field->text()));
            return;
        }
        QString text;
        switch (HexText::toAsciiDraft(field->text(), &text)) {
        case HexText::ToAsciiResult::Ok:
            field->setText(text);
            break;
        case HexText::ToAsciiResult::InvalidHex:
            QToolTip::showText(field->mapToGlobal(QPoint(0, field->height())),
                               tr("Not valid HEX, so it was left unchanged."), field);
            break;
        case HexText::ToAsciiResult::NotText:
            QToolTip::showText(field->mapToGlobal(QPoint(0, field->height())),
                               tr("Contains bytes that can't be shown as text, so it was left unchanged."), field);
            break;
        }
    });
    stepsLayout_->insertWidget(stepsLayout_->count() - 1, row);
}

void BatchEditDialog::removeStep(QWidget *rowWidget) {
    if (stepsLayout_->count() <= 2) return;  // always keep one step (plus the stretch)
    stepsLayout_->removeWidget(rowWidget);
    rowWidget->deleteLater();
    updateSaveEnabled();
}

QVariantList BatchEditDialog::collectSteps() const {
    QVariantList steps;
    for (int i = 0; i < stepsLayout_->count(); ++i) {
        QWidget *row = stepsLayout_->itemAt(i)->widget();
        if (!row || row->property("removed").toBool()) continue;
        const auto *field = row->findChild<QLight::LineEdit *>();
        const auto *hex = row->findChild<QLight::CheckBox *>(QStringLiteral("hex"));
        const auto *crc = row->findChild<QLight::CheckBox *>(QStringLiteral("crc"));
        if (!field) continue;
        steps.append(QVariantMap{
            {QStringLiteral("text"), field->text()},
            {QStringLiteral("isHex"), hex && hex->isChecked()},
            {QStringLiteral("crc"), crc && crc->isChecked()},
        });
    }
    return steps;
}

void BatchEditDialog::updateSaveEnabled() {
    bool anyText = false;
    for (const QVariant &s : collectSteps()) {
        if (!s.toMap().value(QStringLiteral("text")).toString().trimmed().isEmpty()) {
            anyText = true;
            break;
        }
    }
    saveButton_->setEnabled(!nameField_->text().trimmed().isEmpty() && anyText);
    // the delete buttons only make sense with more than one step
    const bool canRemove = stepsLayout_->count() > 2;
    for (int i = 0; i < stepsLayout_->count(); ++i)
        if (QWidget *row = stepsLayout_->itemAt(i)->widget())
            if (auto *del = row->findChild<QLight::ToolButton *>()) del->setEnabled(canRemove);
}

void BatchEditDialog::openForNew() {
    editRow_ = -1;
    nameField_->clear();
    intervalSpin_->setValue(500);
    clearSteps();
    addStep();
    retranslateUi();
    updateSaveEnabled();
    open();
    nameField_->setFocus();
}

void BatchEditDialog::openForEdit(int row, const QString &name, int intervalMs) {
    editRow_ = row;
    nameField_->setText(name);
    intervalSpin_->setValue(intervalMs);
    clearSteps();
    for (const QVariant &v : controller_->stepsForBatchRow(row)) {
        const QVariantMap s = v.toMap();
        addStep(s.value(QStringLiteral("text")).toString(), s.value(QStringLiteral("isHex")).toBool(),
                s.value(QStringLiteral("crc")).toBool());
    }
    if (stepsLayout_->count() <= 1) addStep();
    retranslateUi();
    updateSaveEnabled();
    open();
}

void BatchEditDialog::save() {
    if (!saveButton_->isEnabled()) return;
    const QVariantList steps = collectSteps();
    if (editRow_ >= 0) controller_->updateBatchCommand(editRow_, nameField_->text(), intervalSpin_->value(), steps);
    else controller_->addBatchCommand(nameField_->text(), intervalSpin_->value(), steps);
    accept();
}

}  // namespace ui
