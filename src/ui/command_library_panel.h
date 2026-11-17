// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <QListView>
#include <QStyledItemDelegate>
#include <QWidget>

class AppController;

namespace QLight {
class LineEdit;
class PushButton;
}  // namespace QLight

namespace ui {

class BatchListDialog;
class ComboBox;
class TemplateDialog;
class ThemedLabel;

// Two-line command row: name, and the literal command in monospace below it.
// Selected rows get the accent tint and a marker on the left edge.
class CommandItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit CommandItemDelegate(int nameRole, int detailRole, QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Optional third role painted right-aligned in the secondary colour (history time stamps).
    void setTrailingRole(int role) { trailingRole_ = role; }
    void setRowHeight(int h) { rowHeight_ = h; }
    void setShowMarker(bool on) { showMarker_ = on; }

private:
    int nameRole_;
    int detailRole_;
    int trailingRole_ = -1;
    int rowHeight_ = 50;
    bool showMarker_ = true;
};

// List of device commands. A click (without dragging) emits rowClicked();
// dragging a row (only while reordering is allowed) moves it and emits
// moveRequested(from, to) on release.
class CommandListView : public QListView {
    Q_OBJECT
public:
    explicit CommandListView(QWidget *parent = nullptr);
    void setReorderEnabled(bool on) { reorderEnabled_ = on; }

signals:
    void rowClicked(int row);
    void moveRequested(int from, int to);
    void contextMenuRequested(int row, const QPoint &globalPos);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    int dropRowAt(const QPoint &pos) const;

    bool reorderEnabled_ = true;
    int pressRow_ = -1;
    QPoint pressPos_;
    bool dragging_ = false;
    int dropRow_ = -1;
};

// "Device commands" tab of the left panel.
class CommandLibraryPanel : public QWidget {
    Q_OBJECT
public:
    explicit CommandLibraryPanel(AppController *controller, QWidget *parent = nullptr);

signals:
    // A command with parameters was clicked: the params panel takes over.
    void openParams(int row);

protected:
    void changeEvent(QEvent *e) override;

private:
    void retranslateUi();
    void syncModelCombo();
    void onRowClicked(int row);
    void showRowMenu(int row, const QPoint &globalPos);

    AppController *controller_;
    ThemedLabel *modelLabel_;
    ComboBox *modelCombo_;
    ThemedLabel *descriptionLabel_;
    QLight::LineEdit *searchField_;
    ThemedLabel *dragHint_;
    CommandListView *list_;
    QLight::PushButton *newTemplateButton_;
    QLight::PushButton *batchButton_;
    TemplateDialog *templateDialog_ = nullptr;
    BatchListDialog *batchDialog_ = nullptr;
};

}  // namespace ui
