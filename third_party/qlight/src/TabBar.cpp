#include "QLight/TabBar.h"

#include "QLight/FluentIcon.h"
#include "QLight/Theme.h"

#include <QFocusEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <cmath>

#include "MetricStyle.h"
#include "Util.h"

namespace QLight {

namespace {

constexpr int kTabsHeight = 36;
constexpr int kPivotHeight = 40;
constexpr int kMinTabWidth = 96;
constexpr int kMaxTabWidth = 200;
constexpr int kPadX = 12;
constexpr int kIconSize = 16;
constexpr int kIconGap = 8;
constexpr int kCloseSize = 20;
constexpr int kAddWidth = 32;
constexpr int kAddHeight = 28;
constexpr int kRaise = 4;          // selected tab is raised by this much
constexpr qreal kTabRadius = 8;
constexpr int kPillHeight = 3;

} // namespace

TabBar::TabBar(QWidget *parent) : QTabBar(parent) { init(); }

void TabBar::init()
{
    // Qt's tab layout consults the style for overlaps/shifts; neutralise them.
    auto *st = new detail::MetricStyle;
    st->setParent(this);
    st->setMetric(QStyle::PM_TabBarTabOverlap, 0);
    st->setMetric(QStyle::PM_TabBarBaseOverlap, 0);
    st->setMetric(QStyle::PM_TabBarBaseHeight, 0);
    st->setMetric(QStyle::PM_TabBarTabShiftHorizontal, 0);
    st->setMetric(QStyle::PM_TabBarTabShiftVertical, 0);
    st->setMetric(QStyle::PM_TabBarTabHSpace, 0);
    st->setMetric(QStyle::PM_TabBarTabVSpace, 0);
    setStyle(st);

    setShape(QTabBar::RoundedNorth);
    setDrawBase(false);
    setExpanding(false);
    setUsesScrollButtons(false);
    setElideMode(Qt::ElideRight);
    setMouseTracking(true);
    setFocusPolicy(Qt::TabFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_pillAnim.setDuration(200);
    m_pillAnim.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_pillAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_pill = v.toRectF();
        update();
    });
    connect(this, &QTabBar::currentChanged, this, [this] { animatePillToCurrent(); });
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, qOverload<>(&QWidget::update));
}

int TabBar::barHeight() const { return m_style == Tabs ? kTabsHeight : kPivotHeight; }

void TabBar::setBarStyle(Style s)
{
    if (m_style == s)
        return;
    m_style = s;
    // force Qt to lay the tabs out again with the new size hints
    setIconSize(iconSize().width() == 16 ? QSize(17, 17) : QSize(16, 16));
    setIconSize(QSize(16, 16));
    updateGeometry();
    update();
}

void TabBar::setClosable(bool on)
{
    m_closable = on;
    update();
}

void TabBar::setAddButtonVisible(bool on)
{
    m_addVisible = on;
    updateGeometry();
    update();
}

bool TabBar::showsCloseButton(int index) const
{
    return m_closable && m_style == Tabs && tabRect(index).width() >= 64;
}

QRect TabBar::closeButtonRect(int index) const
{
    if (index < 0 || index >= count() || !showsCloseButton(index))
        return {};
    const QRect r = tabRect(index);
    return QRect(r.right() - 8 - kCloseSize + 1, r.top() + kRaise / 2 + (r.height() - kRaise / 2 - kCloseSize) / 2,
                 kCloseSize, kCloseSize);
}

QRect TabBar::addButtonRect() const
{
    if (!m_addVisible || m_style != Tabs)
        return {};
    const int last = count() - 1;
    const int x = last >= 0 ? tabRect(last).right() + 5 : 4;
    return QRect(x, (height() - kAddHeight) / 2 + kRaise / 2, kAddWidth, kAddHeight);
}

QSize TabBar::tabSizeHint(int index) const
{
    if (m_style == Pivot) {
        // QFontMetrics::horizontalAdvance() rounds; elidedText() compares the exact width,
        // so round up and add slack or the last pixel of a label turns into an ellipsis.
        const QFontMetricsF fm(font());
        int w = 2 * kPadX + int(std::ceil(fm.horizontalAdvance(tabText(index)))) + 2;
        if (!tabIcon(index).isNull())
            w += kIconSize + kIconGap;
        return {w, kPivotHeight};
    }
    const int n = qMax(1, count());
    const int avail = width() - (m_addVisible ? kAddWidth + 8 : 0) - 4;
    return {qBound(kMinTabWidth, avail / n, kMaxTabWidth), kTabsHeight};
}

QSize TabBar::minimumTabSizeHint(int index) const
{
    return m_style == Pivot ? tabSizeHint(index) : QSize(kMinTabWidth, kTabsHeight);
}

QSize TabBar::sizeHint() const
{
    const int addArea = (m_addVisible && m_style == Tabs) ? kAddWidth + 8 : 0;
    if (m_style == Pivot) {
        int w = 0;
        for (int i = 0; i < count(); ++i)
            w += tabSizeHint(i).width();
        return {w, kPivotHeight};
    }
    return {qMax(1, count()) * kMaxTabWidth + addArea, kTabsHeight};
}

QSize TabBar::minimumSizeHint() const
{
    const int addArea = (m_addVisible && m_style == Tabs) ? kAddWidth + 8 : 0;
    return {(m_style == Tabs ? kMinTabWidth : 2 * kPadX) + addArea, barHeight()};
}

TabBar::HitResult TabBar::hitTest(const QPoint &pos) const
{
    if (addButtonRect().contains(pos))
        return {Hit::Add, -1};
    const int i = tabAt(pos);
    if (i < 0)
        return {};
    if (closeButtonRect(i).contains(pos))
        return {Hit::Close, i};
    return {Hit::Tab, i};
}

QRectF TabBar::pillTarget() const
{
    const int i = currentIndex();
    if (i < 0)
        return {};
    const QRect r = tabRect(i);
    const QFontMetrics fm(font());
    const int iconW = tabIcon(i).isNull() ? 0 : kIconSize + kIconGap;
    const qreal contentW = iconW + fm.horizontalAdvance(tabText(i));
    const qreal x = r.left() + (r.width() - contentW) / 2.0;
    return QRectF(x, r.bottom() - kPillHeight - 2, contentW, kPillHeight);
}

void TabBar::animatePillToCurrent()
{
    if (m_style != Pivot)
        return;
    m_pillAnim.stop();
    const QRectF target = pillTarget();
    if (!isVisible() || m_pill.isEmpty()) {
        m_pill = target;
        update();
        return;
    }
    m_pillAnim.setStartValue(m_pill);
    m_pillAnim.setEndValue(target);
    m_pillAnim.start();
}

void TabBar::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal dpr = devicePixelRatioF();
    const qreal bw = detail::hairline(dpr);
    const QFontMetrics fm(font());
    p.setFont(font());

    for (int i = 0; i < count(); ++i) {
        const QRect r = tabRect(i);
        const bool selected = i == currentIndex();
        const bool hovered = m_hover.index == i && m_hover.kind != Hit::None && m_hover.kind != Hit::Add;
        const bool enabled = isTabEnabled(i);

        QRectF content = r; // area the label lives in
        if (m_style == Tabs) {
            if (selected) {
                // raised card-coloured tab, open at the bottom so it merges with the content below
                const QRectF t = QRectF(r).adjusted(bw / 2, kRaise, -bw / 2, 0);
                QPainterPath outline;
                outline.moveTo(t.left(), t.bottom());
                outline.lineTo(t.left(), t.top() + kTabRadius);
                outline.arcTo(QRectF(t.left(), t.top(), 2 * kTabRadius, 2 * kTabRadius), 180, -90);
                outline.lineTo(t.right() - kTabRadius, t.top());
                outline.arcTo(QRectF(t.right() - 2 * kTabRadius, t.top(), 2 * kTabRadius, 2 * kTabRadius), 90, -90);
                outline.lineTo(t.right(), t.bottom());
                QPainterPath fill = outline;
                fill.closeSubpath();
                p.fillPath(fill, pal.cardBg);
                p.setPen(QPen(pal.cardStroke, bw));
                p.setBrush(Qt::NoBrush);
                p.drawPath(outline);
                content = t;
            } else {
                content = QRectF(r).adjusted(0, kRaise, 0, 0);
                if (hovered && enabled) {
                    p.setPen(Qt::NoPen);
                    p.setBrush(pal.subtleFillHover);
                    p.drawRoundedRect(QRectF(r).adjusted(3, kRaise + 3, -3, -3), pal.radiusControl, pal.radiusControl);
                } else if (i + 1 < count() && i + 1 != currentIndex()
                           && !(m_hover.index == i + 1 && m_hover.kind != Hit::None)) {
                    // hairline between two neighbouring unselected tabs
                    p.setPen(QPen(pal.cardStroke, bw));
                    p.drawLine(QPointF(r.right() + 0.5, r.center().y() - 6 + kRaise / 2),
                               QPointF(r.right() + 0.5, r.center().y() + 6 + kRaise / 2));
                }
            }
        }

        // label: icon + elided text
        qreal x = content.left() + kPadX;
        const qreal rightPad = kPadX + (showsCloseButton(i) ? kCloseSize + 4 : 0);
        const QIcon icon = tabIcon(i);
        if (!icon.isNull()) {
            icon.paint(&p, QRect(qRound(x), qRound(content.center().y() - kIconSize / 2.0), kIconSize, kIconSize),
                       Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);
            x += kIconSize + kIconGap;
        }
        QColor textColor = !enabled ? pal.textDisabled : (selected || hovered) ? pal.textPrimary : pal.textSecondary;
        p.setPen(textColor);
        const int avail = qMax(0, qRound(content.right() - rightPad - x));
        p.drawText(QRectF(x, content.top(), avail, content.height()), Qt::AlignVCenter | Qt::AlignLeft,
                   fm.elidedText(tabText(i), Qt::ElideRight, avail));

        // close button
        if (showsCloseButton(i)) {
            const QRect cr = closeButtonRect(i);
            const bool overClose = m_hover.kind == Hit::Close && m_hover.index == i;
            if (overClose) {
                p.setPen(Qt::NoPen);
                p.setBrush(m_pressed.kind == Hit::Close && m_pressed.index == i ? pal.subtleFillPressed : pal.subtleFillHover);
                p.drawRoundedRect(cr, pal.radiusControl, pal.radiusControl);
            }
            const int gs = 12;
            p.drawPixmap(cr.center() - QPoint(gs / 2, gs / 2) + QPoint(0, 0),
                         FluentIcon(Icon::Dismiss).pixmap(gs, selected || overClose ? pal.textPrimary : pal.textSecondary, dpr));
        }
    }

    // "+" button
    if (const QRect ar = addButtonRect(); !ar.isEmpty()) {
        if (m_hover.kind == Hit::Add) {
            p.setPen(Qt::NoPen);
            p.setBrush(m_pressed.kind == Hit::Add ? pal.subtleFillPressed : pal.subtleFillHover);
            p.drawRoundedRect(ar, pal.radiusControl, pal.radiusControl);
        }
        p.drawPixmap(ar.center() - QPoint(8, 8), FluentIcon(Icon::Add).pixmap(16, pal.textPrimary, dpr));
    }

    // Pivot underline (animated between headers)
    if (m_style == Pivot && currentIndex() >= 0) {
        const QRectF pill = m_pillAnim.state() == QAbstractAnimation::Running ? m_pill : pillTarget();
        p.setPen(Qt::NoPen);
        p.setBrush(pal.accent);
        p.drawRoundedRect(pill, kPillHeight / 2.0, kPillHeight / 2.0);
    }

    // keyboard focus ring around the current tab
    if (m_focusVisible && currentIndex() >= 0) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(pal.focusStroke, 2));
        p.drawRoundedRect(QRectF(tabRect(currentIndex())).adjusted(2, kRaise + 1, -2, -1), pal.radiusControl + 1,
                          pal.radiusControl + 1);
    }
}

void TabBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        const HitResult h = hitTest(e->position().toPoint());
        if (h.kind == Hit::Close || h.kind == Hit::Add) {
            m_pressed = h;
            update();
            e->accept();
            return;
        }
    }
    m_pressed = {};
    QTabBar::mousePressEvent(e);
}

void TabBar::mouseMoveEvent(QMouseEvent *e)
{
    const HitResult h = hitTest(e->position().toPoint());
    if (h.kind != m_hover.kind || h.index != m_hover.index) {
        m_hover = h;
        update();
    }
    if (m_pressed.kind == Hit::None)
        QTabBar::mouseMoveEvent(e);
}

void TabBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_pressed.kind != Hit::None) {
        const HitResult pressed = m_pressed;
        m_pressed = {};
        const HitResult h = hitTest(e->position().toPoint());
        update();
        if (h.kind == pressed.kind && h.index == pressed.index) {
            if (pressed.kind == Hit::Close)
                emit tabCloseRequested(pressed.index);
            else
                emit addTabRequested();
        }
        e->accept();
        return;
    }
    QTabBar::mouseReleaseEvent(e);
}

void TabBar::leaveEvent(QEvent *e)
{
    QTabBar::leaveEvent(e);
    m_hover = {};
    update();
}

void TabBar::focusInEvent(QFocusEvent *e)
{
    QTabBar::focusInEvent(e);
    m_focusVisible = detail::isKeyboardFocusReason(e->reason());
    update();
}

void TabBar::focusOutEvent(QFocusEvent *e)
{
    QTabBar::focusOutEvent(e);
    m_focusVisible = false;
    update();
}

void TabBar::resizeEvent(QResizeEvent *e)
{
    QTabBar::resizeEvent(e);
    update();
}

} // namespace QLight
