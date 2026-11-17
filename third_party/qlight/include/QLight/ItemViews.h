#pragma once

#include <QHeaderView>
#include <QListView>
#include <QListWidget>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTableWidget>
#include <QTreeView>
#include <QTreeWidget>

#include "QLight/Export.h"

namespace QLight {

// Item delegate with the Fluent list / grid look: rounded hover and selection fills, an
// accent marker on selected rows, icons, check boxes, hairline row separators (Table mode)
// and QLight editors (LineEdit / SpinBox / DoubleSpinBox) for in-place editing.
//
// The ready-made views below install it for you; use it directly to restyle any other
// QAbstractItemView (a QTreeView, say).
class QLIGHT_EXPORT ItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    enum class Mode {
        List,  // each item is a rounded, inset row
        Table, // cells fill their rect; separators between rows; whole-row hover
        Tree,  // text / icon / check only: TreeView paints the row background, marker and branch arrows
    };

    explicit ItemDelegate(Mode mode = Mode::List, QObject *parent = nullptr);

    Mode mode() const { return m_mode; }
    int rowHeight() const { return m_rowHeight; }
    void setRowHeight(int height) { m_rowHeight = height; }

    // Table mode: the row under the mouse (-1 for none). Maintained by TableView / TableWidget.
    void setHoverRow(int row) { m_hoverRow = row; }
    int hoverRow() const { return m_hoverRow; }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    QRect contentRect(const QRect &cell) const;
    static QRect checkBoxRect(const QRect &content);

    Mode m_mode;
    int m_rowHeight = 36;
    int m_hoverRow = -1;
};

// Column / row headers: transparent, label in the secondary text colour, hairline underneath,
// hover fill and a chevron for the sort indicator.
class QLIGHT_EXPORT HeaderView : public QHeaderView {
    Q_OBJECT
public:
    explicit HeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    QSize sectionSizeFromContents(int logicalIndex) const override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    int m_hover = -1;
};

// QListView / QListWidget with the delegate, overlay scroll bars and a transparent viewport
// (put it on a Card or window background). Single selection by default.
class QLIGHT_EXPORT ListView : public QListView {
    Q_OBJECT
public:
    explicit ListView(QWidget *parent = nullptr);
    ItemDelegate *itemDelegate() const { return m_delegate; }

private:
    ItemDelegate *m_delegate;
};

class QLIGHT_EXPORT ListWidget : public QListWidget {
    Q_OBJECT
public:
    explicit ListWidget(QWidget *parent = nullptr);
    ItemDelegate *itemDelegate() const { return m_delegate; }

private:
    ItemDelegate *m_delegate;
};

// QTableView / QTableWidget as a data grid: HeaderView headers (the row header is hidden by
// default), no grid lines, row selection, 36px rows, whole-row hover, overlay scroll bars.
class QLIGHT_EXPORT TableView : public QTableView {
    Q_OBJECT
public:
    explicit TableView(QWidget *parent = nullptr);
    ItemDelegate *itemDelegate() const { return m_delegate; }

private:
    ItemDelegate *m_delegate;
};

class QLIGHT_EXPORT TableWidget : public QTableWidget {
    Q_OBJECT
public:
    explicit TableWidget(QWidget *parent = nullptr);
    TableWidget(int rows, int columns, QWidget *parent = nullptr);
    ItemDelegate *itemDelegate() const { return m_delegate; }

private:
    void init();
    ItemDelegate *m_delegate;
};

namespace detail { class TreeHover; }

// QTreeView / QTreeWidget: chevron branch indicators, rounded full-row hover and selection with the
// accent marker, 32px rows, no header by default (setHeaderHidden(false) shows a HeaderView).
class QLIGHT_EXPORT TreeView : public QTreeView {
    Q_OBJECT
public:
    explicit TreeView(QWidget *parent = nullptr);
    ItemDelegate *itemDelegate() const { return m_delegate; }

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    ItemDelegate *m_delegate;
    detail::TreeHover *m_hover;
};

class QLIGHT_EXPORT TreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = nullptr);
    ItemDelegate *itemDelegate() const { return m_delegate; }

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    ItemDelegate *m_delegate;
    detail::TreeHover *m_hover;
};

} // namespace QLight
