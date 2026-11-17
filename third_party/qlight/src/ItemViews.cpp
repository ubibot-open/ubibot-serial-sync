#include "QLight/ItemViews.h"

#include "QLight/FluentIcon.h"
#include "QLight/LineEdit.h"
#include "QLight/ScrollBar.h"
#include "QLight/SpinBox.h"
#include "QLight/Theme.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <cmath>
#include <climits>

#include "Field.h"
#include "MetricStyle.h"
#include "Util.h"

namespace QLight {

namespace {

constexpr int kPadX = 12;
constexpr int kIconSize = 16;
constexpr int kIconGap = 10;
constexpr int kCheckSize = 20;
constexpr int kPillW = 3;
constexpr int kPillH = 16;

// Hover row for Table mode: watches the viewport and tells the delegate which row is under the mouse.
class HoverRowTracker : public QObject {
public:
    HoverRowTracker(QAbstractItemView *view, ItemDelegate *delegate)
        : QObject(view), m_view(view), m_delegate(delegate)
    {
        view->viewport()->setMouseTracking(true);
        view->viewport()->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        int row = -2;
        if (e->type() == QEvent::MouseMove)
            row = m_view->indexAt(static_cast<QMouseEvent *>(e)->position().toPoint()).row();
        else if (e->type() == QEvent::Leave)
            row = -1;
        if (row != -2 && row != m_row) {
            m_row = row;
            m_delegate->setHoverRow(row);
            m_view->viewport()->update();
        }
        return false;
    }

private:
    QAbstractItemView *m_view;
    ItemDelegate *m_delegate;
    int m_row = -1;
};

// The views draw selection rows and the focus rectangle through the style before/after the
// delegate runs; suppress both, the delegate paints everything itself.
class ItemViewStyle : public detail::MetricStyle {
public:
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
        if (pe == PE_PanelItemViewRow || pe == PE_PanelItemViewItem || pe == PE_FrameFocusRect)
            return;
        detail::MetricStyle::drawPrimitive(pe, opt, p, w);
    }
};

void skinView(QAbstractItemView *v, ItemDelegate *delegate)
{
    auto *st = new ItemViewStyle;
    st->setParent(v);
    v->setStyle(st);

    v->setFrameShape(QFrame::NoFrame);
    v->viewport()->setAutoFillBackground(false);
    v->setMouseTracking(true);
    v->setItemDelegate(delegate);
    v->setAttribute(Qt::WA_MacShowFocusRect, false);
    v->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    v->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    detail::applyFieldPalette(v);
    ScrollBar::overlayIn(v);
    QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, v, [v] {
        detail::applyFieldPalette(v);
        v->viewport()->update();
    });
}

void skinTree(QTreeView *t, ItemDelegate *delegate)
{
    t->setHeader(new HeaderView(Qt::Horizontal, t));
    skinView(t, delegate);
    t->setHeaderHidden(true);
    t->setIndentation(20);
    t->setUniformRowHeights(true);
    t->setAnimated(true);
    t->setRootIsDecorated(true);
    t->setAllColumnsShowFocus(true);
    t->setExpandsOnDoubleClick(true);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setIconSize(QSize(16, 16));
    t->setTextElideMode(Qt::ElideRight);
    t->header()->setStretchLastSection(true);
    t->header()->setDefaultSectionSize(140);
    t->header()->setHighlightSections(false);
    t->header()->setSectionsClickable(true);
}

void skinTable(QTableView *t, ItemDelegate *delegate)
{
    t->setHorizontalHeader(new HeaderView(Qt::Horizontal, t));
    t->setVerticalHeader(new HeaderView(Qt::Vertical, t));
    skinView(t, delegate);
    new HoverRowTracker(t, delegate);

    t->setShowGrid(false);
    t->setCornerButtonEnabled(false);
    t->setWordWrap(false);
    t->setTextElideMode(Qt::ElideRight);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::ExtendedSelection);
    t->verticalHeader()->hide();
    t->verticalHeader()->setDefaultSectionSize(delegate->rowHeight());
    t->verticalHeader()->setMinimumSectionSize(24);
    t->horizontalHeader()->setDefaultSectionSize(140);
    t->horizontalHeader()->setStretchLastSection(true);
    t->horizontalHeader()->setHighlightSections(false);
    t->horizontalHeader()->setSectionsClickable(true);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

} // namespace

// ---------------------------------------------------------------------------
// ItemDelegate

ItemDelegate::ItemDelegate(Mode mode, QObject *parent) : QStyledItemDelegate(parent), m_mode(mode) {}

QRect ItemDelegate::contentRect(const QRect &cell) const
{
    return m_mode == Mode::List ? cell.adjusted(4, 2, -4, -2) : cell; // Table and Tree use the whole cell
}

QRect ItemDelegate::checkBoxRect(const QRect &content)
{
    return QRect(content.left() + kPadX, content.center().y() - kCheckSize / 2 + 1, kCheckSize, kCheckSize);
}

void ItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    const Palette &pal = ThemeManager::instance().palette();
    const qreal dpr = p->device()->devicePixelRatioF();
    const bool enabled = opt.state & QStyle::State_Enabled;
    const bool selected = opt.state & QStyle::State_Selected;
    const bool hovered = m_mode == Mode::Table ? index.row() == m_hoverRow : bool(opt.state & QStyle::State_MouseOver);
    const QRect cell = contentRect(opt.rect);

    p->save();
    p->setRenderHint(QPainter::Antialiasing);

    // background
    if (m_mode != Mode::Tree && enabled && (selected || hovered)) { // Tree rows are painted by the view
        p->setPen(Qt::NoPen);
        p->setBrush(selected ? pal.subtleFillHover : pal.subtleFillPressed);
        if (m_mode == Mode::List)
            p->drawRoundedRect(QRectF(cell), pal.radiusControl, pal.radiusControl);
        else
            p->drawRect(cell);
    }
    if (m_mode == Mode::Table) {
        p->setPen(QPen(pal.cardStroke, detail::hairline(dpr)));
        p->drawLine(QPointF(cell.left(), cell.bottom() + 0.5), QPointF(cell.right() + 1, cell.bottom() + 0.5));
    }
    if (m_mode != Mode::Tree && selected && enabled && index.column() == 0) { // accent marker on the row's leading edge
        p->setPen(Qt::NoPen);
        p->setBrush(pal.accent);
        p->drawRoundedRect(QRectF(cell.left() + 1, cell.center().y() - kPillH / 2.0 + 0.5, kPillW, kPillH), 1.5, 1.5);
    }

    qreal x = cell.left() + (m_mode == Mode::Tree ? 4 : kPadX);

    // check box
    const QVariant check = index.data(Qt::CheckStateRole);
    if (check.isValid()) {
        const QRect box = checkBoxRect(cell);
        const Qt::CheckState st = Qt::CheckState(check.toInt());
        const bool on = st != Qt::Unchecked;
        const QRectF br = QRectF(box).adjusted(0.5, 0.5, -0.5, -0.5);
        p->setPen(Qt::NoPen);
        p->setBrush(on ? (enabled ? pal.accent : pal.accentDisabled) : Qt::transparent);
        p->drawRoundedRect(br, pal.radiusControl, pal.radiusControl);
        if (!on) {
            p->setPen(QPen(enabled ? pal.controlStrokeStrong : pal.textDisabled, detail::hairline(dpr)));
            p->setBrush(Qt::NoBrush);
            p->drawRoundedRect(br, pal.radiusControl, pal.radiusControl);
        } else {
            p->setPen(QPen(enabled ? pal.textOnAccent : pal.textOnAccentDisabled, 1.6, Qt::SolidLine, Qt::RoundCap,
                           Qt::RoundJoin));
            if (st == Qt::PartiallyChecked) {
                p->drawLine(QPointF(box.left() + 5.5, box.center().y() + 0.5), QPointF(box.right() - 4.5, box.center().y() + 0.5));
            } else {
                QPainterPath path;
                path.moveTo(box.left() + 5.2, box.top() + 10.4);
                path.lineTo(box.left() + 8.6, box.top() + 13.6);
                path.lineTo(box.left() + 14.8, box.top() + 6.6);
                p->drawPath(path);
            }
        }
        x = box.right() + 1 + kIconGap;
    }

    // icon
    const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (!icon.isNull()) {
        icon.paint(p, QRect(qRound(x), cell.center().y() - kIconSize / 2 + 1, kIconSize, kIconSize), Qt::AlignCenter,
                   enabled ? QIcon::Normal : QIcon::Disabled);
        x += kIconSize + kIconGap;
    }

    // text
    const QString text = index.data(Qt::DisplayRole).toString();
    if (!text.isEmpty()) {
        QColor color = enabled ? pal.textPrimary : pal.textDisabled;
        const QVariant fg = index.data(Qt::ForegroundRole);
        if (fg.canConvert<QBrush>() && enabled && fg.value<QBrush>().style() != Qt::NoBrush)
            color = fg.value<QBrush>().color();
        p->setPen(color);
        p->setFont(opt.font);
        const qreal avail = cell.right() - kPadX - x;
        Qt::Alignment align = Qt::AlignVCenter | Qt::AlignLeft;
        const QVariant a = index.data(Qt::TextAlignmentRole);
        if (a.isValid())
            align = Qt::Alignment(a.toInt());
        p->drawText(QRectF(x, cell.top(), avail, cell.height()), int(align),
                    QFontMetricsF(opt.font).elidedText(text, Qt::ElideRight, avail));
    }
    p->restore();
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    const QFontMetricsF fm(opt.font);
    qreal w = 2 * kPadX + fm.horizontalAdvance(index.data(Qt::DisplayRole).toString()) + 2;
    if (index.data(Qt::CheckStateRole).isValid())
        w += kCheckSize + kIconGap;
    if (!index.data(Qt::DecorationRole).value<QIcon>().isNull())
        w += kIconSize + kIconGap;
    return {int(std::ceil(w)), m_rowHeight};
}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QVariant v = index.data(Qt::EditRole);
    switch (v.typeId()) {
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::Short: {
        auto *sb = new SpinBox(parent);
        sb->setRange(INT_MIN, INT_MAX);
        return sb;
    }
    case QMetaType::Double:
    case QMetaType::Float: {
        auto *sb = new DoubleSpinBox(parent);
        sb->setRange(-1e12, 1e12);
        sb->setDecimals(6);
        return sb;
    }
    case QMetaType::QString:
    case QMetaType::QByteArray:
        return new LineEdit(parent);
    default:
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void ItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    QRect r = contentRect(option.rect);
    const int h = qMin(r.height(), editor->sizeHint().height());
    r.setTop(r.top() + (r.height() - h) / 2);
    r.setHeight(h);
    editor->setGeometry(r);
}

bool ItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                               const QModelIndex &index)
{
    const Qt::ItemFlags flags = model->flags(index);
    const bool checkable = (flags & Qt::ItemIsUserCheckable) && (flags & Qt::ItemIsEnabled)
                           && index.data(Qt::CheckStateRole).isValid();
    const auto t = event->type();
    if (checkable && (t == QEvent::MouseButtonRelease || t == QEvent::MouseButtonDblClick
                      || t == QEvent::MouseButtonPress)) {
        // our own check box geometry: do not let the base class apply its style-based one
        const auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() != Qt::LeftButton || !checkBoxRect(contentRect(option.rect)).contains(me->position().toPoint()))
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        if (t == QEvent::MouseButtonRelease) {
            Qt::CheckState next = index.data(Qt::CheckStateRole).toInt() == Qt::Checked ? Qt::Unchecked : Qt::Checked;
            if (flags & Qt::ItemIsAutoTristate)
                next = Qt::CheckState((index.data(Qt::CheckStateRole).toInt() + 1) % 3);
            model->setData(index, next, Qt::CheckStateRole);
        }
        return true;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// ---------------------------------------------------------------------------
// HeaderView

HeaderView::HeaderView(Qt::Orientation orientation, QWidget *parent) : QHeaderView(orientation, parent)
{
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    viewport()->setMouseTracking(true);
    setMouseTracking(true);
    QPalette pal = palette();
    for (auto role : {QPalette::Window, QPalette::Button, QPalette::Base})
        pal.setColor(role, Qt::transparent);
    setPalette(pal);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] { viewport()->update(); });
}

QSize HeaderView::sizeHint() const
{
    const QSize s = QHeaderView::sizeHint();
    return orientation() == Qt::Horizontal ? QSize(s.width(), 36) : QSize(qMax(s.width(), 36), s.height());
}

QSize HeaderView::sectionSizeFromContents(int logicalIndex) const
{
    const QString text = model() ? model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString() : QString();
    const int w = int(std::ceil(QFontMetricsF(font()).horizontalAdvance(text))) + 2 * kPadX + 24; // 24: sort arrow
    return {w, 36};
}

void HeaderView::paintSection(QPainter *p, const QRect &rect, int logicalIndex) const
{
    if (!rect.isValid() || !model())
        return;
    const Palette &pal = ThemeManager::instance().palette();
    const qreal dpr = p->device()->devicePixelRatioF();
    p->save();
    p->setRenderHint(QPainter::Antialiasing);

    if (m_hover == logicalIndex && sectionsClickable() && isEnabled()) {
        p->setPen(Qt::NoPen);
        p->setBrush(pal.subtleFillHover);
        p->drawRect(rect);
    }
    // hairline under (horizontal) / beside (vertical) the section
    p->setPen(QPen(pal.cardStroke, detail::hairline(dpr)));
    if (orientation() == Qt::Horizontal)
        p->drawLine(QPointF(rect.left(), rect.bottom() + 0.5), QPointF(rect.right() + 1, rect.bottom() + 0.5));
    else
        p->drawLine(QPointF(rect.right() + 0.5, rect.top()), QPointF(rect.right() + 0.5, rect.bottom() + 1));

    const QString text = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
    const bool sorted = isSortIndicatorShown() && sortIndicatorSection() == logicalIndex && orientation() == Qt::Horizontal;
    const qreal arrowRoom = sorted ? 22 : 0;

    p->setPen(pal.textSecondary);
    p->setFont(font());
    const qreal avail = rect.width() - 2 * kPadX - arrowRoom;
    Qt::Alignment align = Qt::AlignVCenter | (orientation() == Qt::Horizontal ? Qt::AlignLeft : Qt::AlignCenter);
    p->drawText(QRectF(rect.left() + (orientation() == Qt::Horizontal ? kPadX : 0), rect.top(),
                       orientation() == Qt::Horizontal ? avail : rect.width(), rect.height()),
                int(align), QFontMetricsF(font()).elidedText(text, Qt::ElideRight, avail));

    if (sorted) {
        const Icon icon = sortIndicatorOrder() == Qt::AscendingOrder ? Icon::ChevronUp : Icon::ChevronDown;
        p->drawPixmap(rect.right() - kPadX - 10, rect.center().y() - 5, FluentIcon(icon).pixmap(10, pal.textSecondary, dpr));
    }
    p->restore();
}

void HeaderView::mouseMoveEvent(QMouseEvent *e)
{
    const int i = logicalIndexAt(e->position().toPoint());
    if (i != m_hover) {
        m_hover = i;
        viewport()->update();
    }
    QHeaderView::mouseMoveEvent(e);
}

void HeaderView::leaveEvent(QEvent *e)
{
    QHeaderView::leaveEvent(e);
    m_hover = -1;
    viewport()->update();
}

// ---------------------------------------------------------------------------
// Ready-made views

ListView::ListView(QWidget *parent) : QListView(parent), m_delegate(new ItemDelegate(ItemDelegate::Mode::List, this))
{
    skinView(this, m_delegate);
    setUniformItemSizes(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

ListWidget::ListWidget(QWidget *parent)
    : QListWidget(parent), m_delegate(new ItemDelegate(ItemDelegate::Mode::List, this))
{
    skinView(this, m_delegate);
    setUniformItemSizes(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

TableView::TableView(QWidget *parent)
    : QTableView(parent), m_delegate(new ItemDelegate(ItemDelegate::Mode::Table, this))
{
    skinTable(this, m_delegate);
}

TableWidget::TableWidget(QWidget *parent)
    : QTableWidget(parent), m_delegate(new ItemDelegate(ItemDelegate::Mode::Table, this))
{
    init();
}

TableWidget::TableWidget(int rows, int columns, QWidget *parent)
    : QTableWidget(rows, columns, parent), m_delegate(new ItemDelegate(ItemDelegate::Mode::Table, this))
{
    init();
}

void TableWidget::init() { skinTable(this, m_delegate); }

// ---------------------------------------------------------------------------
// TreeView / TreeWidget

namespace detail {

// Tracks the item under the mouse so the whole row (not just the cell) can be highlighted.
class TreeHover : public QObject {
public:
    explicit TreeHover(QTreeView *view) : QObject(view), m_view(view)
    {
        view->viewport()->setMouseTracking(true);
        view->viewport()->installEventFilter(this);
    }

    bool isHovered(const QModelIndex &idx) const
    {
        if (!m_index.isValid() || !idx.isValid())
            return false;
        const QModelIndex h(m_index);
        return idx.row() == h.row() && idx.parent() == h.parent();
    }

protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() == QEvent::MouseMove) {
            const QModelIndex idx = m_view->indexAt(static_cast<QMouseEvent *>(e)->position().toPoint());
            const QPersistentModelIndex now = idx.isValid() ? QPersistentModelIndex(idx.siblingAtColumn(0)) : QPersistentModelIndex();
            if (now != m_index) {
                m_index = now;
                m_view->viewport()->update();
            }
        } else if (e->type() == QEvent::Leave && m_index.isValid()) {
            m_index = QPersistentModelIndex();
            m_view->viewport()->update();
        }
        return false;
    }

private:
    QTreeView *m_view;
    QPersistentModelIndex m_index;
};

} // namespace detail

namespace {

void paintTreeRow(const QTreeView *v, const detail::TreeHover *hover, QPainter *p, const QStyleOptionViewItem &opt,
                  const QModelIndex &index)
{
    const Palette &pal = ThemeManager::instance().palette();
    // (the option passed to drawRow() carries no per-item selection state: ask the model)
    const bool enabled = v->isEnabled();
    const bool selected = v->selectionModel() && v->selectionModel()->isSelected(index.siblingAtColumn(0));
    const bool hovered = hover && hover->isHovered(index);
    if (!enabled || !(selected || hovered))
        return;
    const QRect row(4, opt.rect.y() + 2, v->viewport()->width() - 8, opt.rect.height() - 4);
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(selected ? pal.subtleFillHover : pal.subtleFillPressed);
    p->drawRoundedRect(QRectF(row), pal.radiusControl, pal.radiusControl);
    if (selected) {
        p->setBrush(pal.accent);
        p->drawRoundedRect(QRectF(row.left() + 1, row.center().y() - kPillH / 2.0 + 0.5, kPillW, kPillH), 1.5, 1.5);
    }
    p->restore();
}

void paintBranch(const QTreeView *v, QPainter *p, const QRect &rect, const QModelIndex &index)
{
    if (!index.model() || !index.model()->hasChildren(index))
        return;
    const Palette &pal = ThemeManager::instance().palette();
    const int ind = v->indentation();
    const QRect cell(rect.right() - ind + 1, rect.y(), ind, rect.height());
    const Icon icon = v->isExpanded(index) ? Icon::ChevronDown : Icon::ChevronRight;
    p->drawPixmap(cell.center() - QPoint(6, 6),
                  FluentIcon(icon).pixmap(12, pal.textSecondary, p->device()->devicePixelRatioF()));
}

} // namespace

TreeView::TreeView(QWidget *parent)
    : QTreeView(parent), m_delegate(new ItemDelegate(ItemDelegate::Mode::Tree, this)), m_hover(nullptr)
{
    skinTree(this, m_delegate);
    m_delegate->setRowHeight(32);
    m_hover = new detail::TreeHover(this);
}

void TreeView::drawBranches(QPainter *p, const QRect &rect, const QModelIndex &index) const { paintBranch(this, p, rect, index); }

void TreeView::drawRow(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    paintTreeRow(this, m_hover, p, opt, index);
    QTreeView::drawRow(p, opt, index);
}

TreeWidget::TreeWidget(QWidget *parent)
    : QTreeWidget(parent), m_delegate(new ItemDelegate(ItemDelegate::Mode::Tree, this)), m_hover(nullptr)
{
    skinTree(this, m_delegate);
    m_delegate->setRowHeight(32);
    m_hover = new detail::TreeHover(this);
}

void TreeWidget::drawBranches(QPainter *p, const QRect &rect, const QModelIndex &index) const { paintBranch(this, p, rect, index); }

void TreeWidget::drawRow(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    paintTreeRow(this, m_hover, p, opt, index);
    QTreeWidget::drawRow(p, opt, index);
}

} // namespace QLight
