#include "ui/flow_layout.h"

#include <QWidget>

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), hSpace_(hSpacing), vSpace_(vSpacing) {
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int hSpacing, int vSpacing) : hSpace_(hSpacing), vSpace_(vSpacing) {}

FlowLayout::~FlowLayout() {
    QLayoutItem *item;
    while ((item = takeAt(0))) delete item;
}

void FlowLayout::addItem(QLayoutItem *item) { itemList_.append(item); }

int FlowLayout::horizontalSpacing() const {
    if (hSpace_ >= 0) return hSpace_;
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const {
    if (vSpace_ >= 0) return vSpace_;
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::count() const { return itemList_.size(); }

QLayoutItem *FlowLayout::itemAt(int index) const { return itemList_.value(index); }

QLayoutItem *FlowLayout::takeAt(int index) {
    if (index >= 0 && index < itemList_.size()) return itemList_.takeAt(index);
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const { return {}; }

bool FlowLayout::hasHeightForWidth() const { return true; }

int FlowLayout::heightForWidth(int width) const { return doLayout(QRect(0, 0, width, 0), true); }

void FlowLayout::setGeometry(const QRect &rect) {
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const { return minimumSize(); }

QSize FlowLayout::minimumSize() const {
    QSize size;
    for (QLayoutItem *item : itemList_) size = size.expandedTo(item->minimumSize());
    const QMargins m = contentsMargins();
    size += QSize(m.left() + m.right(), m.top() + m.bottom());
    return size;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const {
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(left, top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (QLayoutItem *item : itemList_) {
        QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1) spaceX = wid ? wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                                                      QSizePolicy::PushButton, Qt::Horizontal)
                                        : 0;
        int spaceY = verticalSpacing();
        if (spaceY == -1) spaceY = wid ? wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                                                      QSizePolicy::PushButton, Qt::Vertical)
                                        : 0;

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly) item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const {
    QObject *p = parent();
    if (!p) return -1;
    if (p->isWidgetType()) {
        auto *pw = static_cast<QWidget *>(p);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout *>(p)->spacing();
}
