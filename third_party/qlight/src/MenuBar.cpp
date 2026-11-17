#include "QLight/MenuBar.h"

#include "QLight/Menu.h"
#include "QLight/Theme.h"

#include <QApplication>
#include <QEvent>
#include <QFontMetricsF>
#include <QPainter>
#include <QStyleOptionMenuItem>

#include <cmath>

#include "ToolStyle.h"
#include "Util.h"

namespace QLight {

namespace {

constexpr int kItemHeight = 32;
constexpr int kItemPad = 12;
constexpr int kBarHeight = 36;
constexpr int kStatusHeight = 28;

// Item sizes and spacing for the menu bar; derives from the tool-button style so the overflow
// ("extension") button that QMenuBar creates can use the very same style object type.
class MenuBarStyle : public detail::ToolButtonStyle {
public:
    bool compact = false; // item height 28 (bar 32) instead of 32 (bar 36)

    MenuBarStyle()
    {
        setMetric(PM_MenuBarHMargin, 4);
        setMetric(PM_MenuBarVMargin, 2);
        setMetric(PM_MenuBarItemSpacing, 2);
        setMetric(PM_MenuBarPanelWidth, 0);
        setHint(SH_MenuBar_AltKeyNavigation, 1);
        setHint(SH_MenuBar_MouseTracking, 1);
    }

    QSize sizeFromContents(ContentsType type, const QStyleOption *opt, const QSize &csz, const QWidget *w) const override
    {
        if (type != CT_MenuBarItem)
            return detail::ToolButtonStyle::sizeFromContents(type, opt, csz, w);
        const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
        const QString text = mi ? detail::plainText(mi->text) : QString();
        const int tw = mi ? int(std::ceil(QFontMetricsF(mi->font).horizontalAdvance(text))) + 2 : csz.width();
        return {tw + 2 * kItemPad, compact ? kItemHeight - 4 : kItemHeight};
    }

    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
        if (ce == CE_MenuBarItem || ce == CE_MenuBarEmptyArea)
            return; // MenuBar::paintEvent draws the items
        detail::ToolButtonStyle::drawControl(ce, opt, p, w);
    }

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
        if (pe == PE_PanelMenuBar || pe == PE_FrameMenu)
            return;
        detail::ToolButtonStyle::drawPrimitive(pe, opt, p, w);
    }
};

} // namespace

MenuBar::MenuBar(QWidget *parent) : QMenuBar(parent) { init(); }

void MenuBar::init()
{
    auto *st = new MenuBarStyle;
    st->setParent(this);
    setStyle(st);
    setNativeMenuBar(false);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setMinimumHeight(kBarHeight);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, qOverload<>(&QWidget::update));
}

void MenuBar::setCompact(bool compact)
{
    m_compact = compact;
    if (auto *st = dynamic_cast<MenuBarStyle *>(style()))
        st->compact = compact;
    setMinimumHeight(compact ? kBarHeight - 4 : kBarHeight);
    updateGeometry();
    update();
}

Menu *MenuBar::createMenu(const QString &title)
{
    auto *m = new Menu(title, this);
    addMenu(m);
    return m;
}

// The overflow button QMenuBar creates when the items do not fit is a plain QToolButton.
void MenuBar::styleChildren()
{
    for (auto *b : findChildren<QToolButton *>()) {
        if (!dynamic_cast<detail::ToolButtonStyle *>(b->style())) {
            auto *st = new detail::ToolButtonStyle;
            st->setParent(b);
            b->setStyle(st);
            b->setAttribute(Qt::WA_Hover);
        }
    }
}

bool MenuBar::event(QEvent *e)
{
    const bool handled = QMenuBar::event(e);
    if (e->type() == QEvent::ChildPolished || e->type() == QEvent::Show || e->type() == QEvent::LayoutRequest)
        styleChildren();
    return handled;
}

void MenuBar::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setFont(font());

    for (QAction *a : actions()) {
        if (!a->isVisible() || a->isSeparator())
            continue;
        const QRect r = actionGeometry(a);
        const bool enabled = a->isEnabled();
        const qreal inset = qMax(0, r.height() - kItemHeight) / 2.0; // compact bars use the item rect as it is
        const QRectF item = QRectF(r).adjusted(0, inset, 0, -inset);
        if (enabled && a == activeAction()) {
            p.setPen(Qt::NoPen);
            p.setBrush(pal.subtleFillHover);
            p.drawRoundedRect(item, pal.radiusControl, pal.radiusControl);
        }
        p.setPen(enabled ? pal.textPrimary : pal.textDisabled);
        p.drawText(item, Qt::AlignCenter, detail::plainText(a->text()));
    }
}

// ---------------------------------------------------------------------------

StatusBar::StatusBar(QWidget *parent) : QStatusBar(parent) { init(); }

void StatusBar::init()
{
    setSizeGripEnabled(false);
    setAutoFillBackground(false);
    setMinimumHeight(kStatusHeight);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, qOverload<>(&QWidget::update));
    connect(this, &QStatusBar::messageChanged, this, qOverload<>(&QWidget::update));
}

QSize StatusBar::sizeHint() const { return {QStatusBar::sizeHint().width(), kStatusHeight}; }

void StatusBar::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    const qreal bw = detail::hairline(devicePixelRatioF());
    p.setPen(QPen(pal.cardStroke, bw));
    p.drawLine(QPointF(0, bw / 2), QPointF(width(), bw / 2));

    const QString msg = currentMessage();
    if (msg.isEmpty())
        return;
    // the message is drawn left of the first visible child widget
    int right = width() - 12;
    for (QWidget *c : findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly))
        if (c->isVisible() && c->x() > 0)
            right = qMin(right, c->x() - 8);
    p.setPen(pal.textSecondary);
    p.setFont(font());
    p.drawText(QRectF(12, 0, qMax(0, right - 12), height()), Qt::AlignVCenter | Qt::AlignLeft,
               QFontMetricsF(font()).elidedText(msg, Qt::ElideRight, qMax(0, right - 12)));
}

} // namespace QLight
