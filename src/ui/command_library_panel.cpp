// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/command_library_panel.h"

#include "app/app_controller.h"
#include "ui/batch_dialogs.h"
#include "ui/template_dialog.h"
#include "ui/ui_common.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

#include <QLight/FluentIcon.h>
#include <QLight/LineEdit.h>
#include <QLight/Menu.h>
#include <QLight/PushButton.h>
#include <QLight/ScrollBar.h>
#include <QLight/Theme.h>

namespace ui {

// --- CommandItemDelegate -------------------------------------------------

CommandItemDelegate::CommandItemDelegate(int nameRole, int detailRole, QObject *parent)
    : QStyledItemDelegate(parent), nameRole_(nameRole), detailRole_(detailRole) {}

QSize CommandItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const {
    return QSize(option.rect.width(), rowHeight_);
}

void CommandItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    const AppColors &c = colors();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const QRectF row = QRectF(option.rect).adjusted(0, 1, 0, -1);
    const bool selected = option.state & QStyle::State_Selected;
    const bool hovered = option.state & QStyle::State_MouseOver;
    if (selected || hovered) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(selected ? c.accentTint : p.subtleFillHover);
        painter->drawRoundedRect(row, p.radiusControl, p.radiusControl);
    }
    if (selected && showMarker_) {
        painter->setBrush(p.accent);
        const qreal h = qMin<qreal>(row.height() - 12, 28);
        painter->drawRoundedRect(QRectF(row.left(), row.center().y() - h / 2, 3, h), 1.5, 1.5);
    }

    const QRect content = option.rect.adjusted(12, 0, -12, 0);
    const QString name = index.data(nameRole_).toString();
    const QString detail = detailRole_ >= 0 ? index.data(detailRole_).toString() : QString();
    const QString trailing = trailingRole_ >= 0 ? index.data(trailingRole_).toString() : QString();

    QFont nameFont = option.font;
    if (selected) nameFont.setWeight(QFont::DemiBold);
    const QFontMetrics nfm(nameFont);
    QFont detailFont = monoFont(qMax(10, (option.font.pixelSize() > 0 ? option.font.pixelSize() : 13) - 2));
    const QFontMetrics dfm(detailFont);

    int trailingWidth = 0;
    if (!trailing.isEmpty()) {
        QFont tf = option.font;
        if (tf.pixelSize() > 0) tf.setPixelSize(tf.pixelSize() - 1);
        trailingWidth = QFontMetrics(tf).horizontalAdvance(trailing) + 12;
        painter->setFont(tf);
        painter->setPen(p.textSecondary);
        painter->drawText(content, Qt::AlignRight | Qt::AlignVCenter, trailing);
    }
    const QRect textRect = content.adjusted(0, 0, -trailingWidth, 0);

    if (detail.isEmpty()) {
        painter->setFont(nameFont);
        painter->setPen(p.textPrimary);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                          nfm.elidedText(name, Qt::ElideRight, textRect.width()));
    } else {
        const int total = nfm.height() + dfm.height() + 1;
        const int top = content.top() + (content.height() - total) / 2;
        painter->setFont(nameFont);
        painter->setPen(p.textPrimary);
        painter->drawText(QRect(textRect.left(), top, textRect.width(), nfm.height()), Qt::AlignLeft | Qt::AlignVCenter,
                          nfm.elidedText(name, Qt::ElideRight, textRect.width()));
        painter->setFont(detailFont);
        painter->setPen(selected ? c.accentText : p.textSecondary);
        painter->drawText(QRect(textRect.left(), top + nfm.height() + 1, textRect.width(), dfm.height()),
                          Qt::AlignLeft | Qt::AlignVCenter, dfm.elidedText(detail, Qt::ElideRight, textRect.width()));
    }
    painter->restore();
}

// --- CommandListView -----------------------------------------------------

CommandListView::CommandListView(QWidget *parent) : QListView(parent) {
    setFrameShape(QFrame::NoFrame);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);
    viewport()->setAttribute(Qt::WA_Hover);
    viewport()->setAutoFillBackground(false);
    setStyleSheet(QStringLiteral("QListView { background: transparent; }"));
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setUniformItemSizes(true);
    QLight::ScrollBar::overlayIn(this);
}

int CommandListView::dropRowAt(const QPoint &pos) const {
    const int count = model() ? model()->rowCount() : 0;
    if (count == 0) return -1;
    const QModelIndex idx = indexAt(QPoint(4, pos.y()));
    if (idx.isValid()) return idx.row();
    return pos.y() < 0 ? 0 : count - 1;
}

void CommandListView::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        const QModelIndex idx = indexAt(e->position().toPoint());
        pressRow_ = idx.isValid() ? idx.row() : -1;
        pressPos_ = e->position().toPoint();
        dragging_ = false;
        dropRow_ = -1;
        if (idx.isValid()) setCurrentIndex(idx);
        return;
    }
    QListView::mousePressEvent(e);
}

void CommandListView::mouseMoveEvent(QMouseEvent *e) {
    if ((e->buttons() & Qt::LeftButton) && pressRow_ >= 0 && reorderEnabled_) {
        if (!dragging_ && (e->position().toPoint() - pressPos_).manhattanLength() >= QApplication::startDragDistance()) {
            dragging_ = true;
            setCursor(Qt::ClosedHandCursor);
        }
        if (dragging_) {
            dropRow_ = dropRowAt(e->position().toPoint());
            viewport()->update();
            return;
        }
    }
    QListView::mouseMoveEvent(e);
}

void CommandListView::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton) {
        QListView::mouseReleaseEvent(e);
        return;
    }
    const int from = pressRow_;
    pressRow_ = -1;
    if (dragging_) {
        dragging_ = false;
        unsetCursor();
        const int to = dropRow_;
        dropRow_ = -1;
        viewport()->update();
        if (from >= 0 && to >= 0 && to != from) emit moveRequested(from, to);
        return;
    }
    const QModelIndex idx = indexAt(e->position().toPoint());
    if (from >= 0 && idx.isValid() && idx.row() == from) emit rowClicked(from);
}

void CommandListView::contextMenuEvent(QContextMenuEvent *e) {
    const QModelIndex idx = indexAt(e->pos());
    if (idx.isValid()) emit contextMenuRequested(idx.row(), e->globalPos());
}

void CommandListView::leaveEvent(QEvent *e) {
    QListView::leaveEvent(e);
    viewport()->update();
}

void CommandListView::paintEvent(QPaintEvent *e) {
    QListView::paintEvent(e);
    if (!dragging_ || dropRow_ < 0 || !model()) return;
    // insertion marker where the dragged row will land
    const QRect target = visualRect(model()->index(dropRow_, 0));
    const int y = dropRow_ > pressRow_ ? target.bottom() : target.top();
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QLight::ThemeManager::instance().palette().accent);
    painter.drawRoundedRect(QRectF(target.left() + 4, y - 1, target.width() - 8, 2), 1, 1);
}

// --- CommandLibraryPanel -------------------------------------------------

CommandLibraryPanel::CommandLibraryPanel(AppController *controller, QWidget *parent)
    : QWidget(parent), controller_(controller) {
    auto *col = new QVBoxLayout(this);
    col->setContentsMargins(16, 14, 16, 16);
    col->setSpacing(8);

    modelLabel_ = new ThemedLabel(ThemedLabel::Primary, this);
    modelLabel_->setEmphasis(true);
    col->addWidget(modelLabel_);
    modelCombo_ = new ComboBox(this);
    modelCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    modelCombo_->setMinimumContentsLength(8);
    col->addWidget(modelCombo_);
    descriptionLabel_ = new ThemedLabel(ThemedLabel::Secondary, this);
    descriptionLabel_->setWordWrap(true);
    col->addWidget(descriptionLabel_);

    col->addSpacing(6);
    searchField_ = new QLight::LineEdit(this);
    searchField_->setClearButtonEnabled(true);
    col->addWidget(searchField_);
    dragHint_ = new ThemedLabel(ThemedLabel::Secondary, this);
    dragHint_->setEmphasis(false, -1);
    col->addWidget(dragHint_);

    list_ = new CommandListView(this);
    list_->setModel(controller_->commandModel());
    list_->setItemDelegate(new CommandItemDelegate(CommandListModel::NameRole, CommandListModel::CmdRole, list_));
    col->addWidget(list_, 1);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(12);
    newTemplateButton_ = new QLight::PushButton(this);
    newTemplateButton_->setFluentIcon(QLight::Icon::Add);
    batchButton_ = new QLight::PushButton(this);
    newTemplateButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    batchButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttons->addWidget(newTemplateButton_);
    buttons->addWidget(batchButton_);
    col->addLayout(buttons);

    CommandListModel *commands = controller_->commandModel();
    searchField_->setText(commands->searchText());
    connect(searchField_, &QLineEdit::textChanged, this, [this, commands](const QString &text) {
        commands->setSearchText(text);
        list_->setReorderEnabled(text.isEmpty());
        dragHint_->setVisible(text.isEmpty());
    });
    connect(list_, &CommandListView::rowClicked, this, &CommandLibraryPanel::onRowClicked);
    connect(list_, &CommandListView::moveRequested, controller_, &AppController::moveCommandRow);
    connect(list_, &CommandListView::contextMenuRequested, this, &CommandLibraryPanel::showRowMenu);

    connect(modelCombo_, &QComboBox::activated, this,
            [this](int i) { controller_->setCurrentModelId(modelCombo_->itemText(i)); });
    connect(controller_, &AppController::currentModelChanged, this, &CommandLibraryPanel::syncModelCombo);
    connect(controller_, &AppController::libraryChanged, this, &CommandLibraryPanel::syncModelCombo);

    connect(newTemplateButton_, &QPushButton::clicked, this, [this] {
        if (!templateDialog_) templateDialog_ = new TemplateDialog(controller_, window());
        templateDialog_->openForNew();
    });
    connect(batchButton_, &QPushButton::clicked, this, [this] {
        if (!batchDialog_) batchDialog_ = new BatchListDialog(controller_, window());
        batchDialog_->open();
    });

    syncModelCombo();
    retranslateUi();
}

void CommandLibraryPanel::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) retranslateUi();
}

void CommandLibraryPanel::retranslateUi() {
    modelLabel_->setText(tr("Device model"));
    searchField_->setPlaceholderText(tr("Search commands"));
    dragHint_->setText(tr("Drag a row to reorder"));
    newTemplateButton_->setText(tr("New template"));
    batchButton_->setText(tr("Batch commands"));
    // the description is localised by the library itself
    descriptionLabel_->setText(controller_->currentModelDescription());
}

void CommandLibraryPanel::syncModelCombo() {
    modelCombo_->blockSignals(true);
    const QStringList ids = controller_->modelIds();
    if (modelCombo_->count() != ids.size()) {
        modelCombo_->clear();
        modelCombo_->addItems(ids);
    } else {
        for (int i = 0; i < ids.size(); ++i) modelCombo_->setItemText(i, ids.at(i));
    }
    modelCombo_->setCurrentIndex(ids.indexOf(controller_->currentModelId()));
    modelCombo_->blockSignals(false);
    descriptionLabel_->setText(controller_->currentModelDescription());
    descriptionLabel_->setVisible(!descriptionLabel_->text().isEmpty());
}

void CommandLibraryPanel::onRowClicked(int row) {
    const QModelIndex idx = controller_->commandModel()->index(row);
    if (idx.data(CommandListModel::HasParamsRole).toBool()) emit openParams(row);
    else controller_->loadCommandIntoDraft(row);
}

void CommandLibraryPanel::showRowMenu(int row, const QPoint &globalPos) {
    const QModelIndex idx = controller_->commandModel()->index(row);
    if (!idx.data(CommandListModel::IsCustomRole).toBool()) return;
    list_->setCurrentIndex(idx);
    QLight::Menu menu(this);
    QAction *edit = menu.addAction(QLight::FluentIcon(QLight::Icon::Edit).icon(), tr("Edit"));
    QAction *remove = menu.addAction(QLight::FluentIcon(QLight::Icon::Delete).icon(), tr("Delete"));
    QAction *chosen = menu.exec(globalPos);
    if (chosen == edit) {
        if (!templateDialog_) templateDialog_ = new TemplateDialog(controller_, window());
        templateDialog_->openForEdit(row, idx.data(CommandListModel::NameRole).toString(),
                                     idx.data(CommandListModel::CmdRole).toString());
    } else if (chosen == remove) {
        controller_->removeCustomTemplate(row);
    }
}

}  // namespace ui
