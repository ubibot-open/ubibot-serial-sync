#include "QLight/Menu.h"

#include "QLight/FluentIcon.h"
#include "QLight/Shadow.h"
#include "QLight/Theme.h"

#include <QActionGroup>
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QScreen>
#include <QStyleOption>

#include "MetricStyle.h"
#include "Util.h"

#include <cmath>

namespace QLight {

namespace {

constexpr int kItemHeight = 36;   // row pitch; the visible highlight is 4px shorter
constexpr int kSeparatorHeight = 9;
constexpr int kMinItemWidth = 160;
constexpr int kPadLeft = 12;
constexpr int kPadRight = 12;
constexpr int kIconSize = 16;
constexpr int kIconColumn = kIconSize + 10;
constexpr int kShortcutGap = 24;
constexpr int kArrowSize = 12;
constexpr int kArrowColumn = kArrowSize + 8;
constexpr int kCardPad = 4;       // between the card edge and the items

ShadowSpec menuShadow()
{
    const Palette &p = ThemeManager::instance().palette();
    ShadowSpec s;
    s.radius = p.radiusCard;
    s.blur = 10;
    s.offsetY = 3;
    s.color = p.shadowColor;
    s.color.setAlpha(qMin(255, s.color.alpha() * 3 / 2));
    return s;
}

class MenuStyle : public detail::MetricStyle {
public:
    MenuStyle()
    {
        const int m = Menu::shadowMargin();
        setMetric(PM_MenuHMargin, m + kCardPad);
        setMetric(PM_MenuVMargin, m + kCardPad);
        setMetric(PM_MenuPanelWidth, 0);
        setMetric(PM_MenuDesktopFrameWidth, 0);
        setMetric(PM_SmallIconSize, kIconSize);
        setHint(SH_Menu_Scrollable, 0);
        setHint(SH_Menu_SubMenuPopupDelay, 200);
        setHint(SH_Menu_SloppySubMenus, 1);
        setHint(SH_Menu_MouseTracking, 1);
        setHint(SH_Menu_SpaceActivatesItem, 1);
        setHint(SH_Menu_FlashTriggeredItem, 0);
        setHint(SH_Menu_KeyboardSearch, 1);
        setHint(SH_Menu_AllowActiveAndDisabled, 0);
        setHint(SH_Menu_Mask, 0);
    }

    QSize sizeFromContents(ContentsType type, const QStyleOption *opt, const QSize &csz,
                           const QWidget *w) const override
    {
        if (type != CT_MenuItem)
            return detail::MetricStyle::sizeFromContents(type, opt, csz, w);
        const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
        if (!mi)
            return csz;
        if (mi->menuItemType == QStyleOptionMenuItem::Separator)
            return {csz.width(), kSeparatorHeight};

        const QString label = detail::plainText(mi->text.section(QLatin1Char('\t'), 0, 0));
        const QString shortcut = mi->text.section(QLatin1Char('\t'), 1);
        // round up + slack: the label is later elided against the exact text width
        const QFontMetricsF fm(mi->font);
        int width = kPadLeft + int(std::ceil(fm.horizontalAdvance(label))) + 2;
        if (mi->maxIconWidth > 0 || mi->menuHasCheckableItems)
            width += kIconColumn;
        if (!shortcut.isEmpty())
            width += kShortcutGap + int(std::ceil(fm.horizontalAdvance(shortcut))) + 2;
        width += mi->menuItemType == QStyleOptionMenuItem::SubMenu ? kArrowColumn : kPadRight;
        return {qMax(width, kMinItemWidth), kItemHeight};
    }
};

} // namespace

int Menu::shadowMargin() { return int(Shadow::extent(menuShadow())) + 2; }

Menu::Menu(QWidget *parent) : QMenu(parent) { init(); }
Menu::Menu(const QString &title, QWidget *parent) : QMenu(title, parent) { init(); }

void Menu::init()
{
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    auto *st = new MenuStyle;
    st->setParent(this);
    setStyle(st);
    setFont(QApplication::font());
}

Menu *Menu::addSubMenu(const QString &title, const QIcon &icon)
{
    auto *sub = new Menu(title, this);
    if (!icon.isNull())
        sub->menuAction()->setIcon(icon);
    addMenu(sub);
    return sub;
}

QMenu *Menu::parentMenu() const
{
    // UbiBot patch: only a *visible* menu can be the one this menu opens from. A menu bar's
    // drop-down menus can also be associated with the bar's hidden overflow ("»") menu (Qt
    // moves items there while the bar is too narrow, e.g. before the first layout); upstream
    // then treated them as sub-menus of that hidden menu and positioned them against it,
    // which put every drop-down except the first one at the top-left of the screen.
    for (QObject *o : menuAction()->associatedObjects())
        if (o != this)
            if (auto *m = qobject_cast<QMenu *>(o); m && m->isVisible())
                return m;
    return nullptr;
}

bool Menu::isSubMenu() const { return parentMenu() != nullptr; }

bool Menu::hasIconColumn() const
{
    for (QAction *a : actions())
        if (a->isVisible() && !a->isSeparator() && (!a->icon().isNull() || a->isCheckable()))
            return true;
    return false;
}

void Menu::showEvent(QShowEvent *e)
{
    QMenu::showEvent(e);
    if (m_shifted)
        return;
    m_shifted = true;
    const int m = shadowMargin();

    if (QMenu *parent = parentMenu()) {
        // Sub-menu: its card sits against the parent's card, level with the parent item.
        // (Qt's own placement does not know that our widgets have a shadow margin.)
        const QRect item = parent->actionGeometry(menuAction());
        const QPoint parentCardTopRight = parent->mapToGlobal(QPoint(parent->width() - m, 0));
        const QPoint parentCardTopLeft = parent->mapToGlobal(QPoint(m, 0));
        const QRect avail = screen()->availableGeometry();
        const int cardW = width() - 2 * m;
        const int y = parent->mapToGlobal(QPoint(0, item.top())).y() - kCardPad - 1;

        int x = parentCardTopRight.x() - 2; // slight overlap reads as "attached"
        if (x + cardW > avail.right())        // no room on the right: open to the left
            x = parentCardTopLeft.x() - cardW + 2;
        int yy = y;
        const int cardH = height() - 2 * m;
        if (yy + cardH > avail.bottom())
            yy = qMax(avail.top(), avail.bottom() - cardH);
        move(x - m, yy - m);
    } else {
        // Qt puts the widget's top-left at the requested point; the visible card starts
        // shadowMargin() further in, so shift the top-level menu to compensate.
        move(pos() - QPoint(m, m));
    }
}

void Menu::hideEvent(QHideEvent *e)
{
    QMenu::hideEvent(e);
    m_shifted = false;
}

void Menu::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal dpr = devicePixelRatioF();
    const qreal bw = detail::hairline(dpr);
    const int m = shadowMargin();
    const QRectF card = QRectF(rect()).adjusted(m, m, -m, -m);

    Shadow::draw(&p, card, menuShadow(), dpr);
    p.setBrush(pal.cardBg);
    p.setPen(QPen(pal.cardStroke, bw));
    p.drawRoundedRect(card.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), pal.radiusCard, pal.radiusCard);

    const bool iconCol = hasIconColumn();
    const QFontMetrics fm(font());

    for (QAction *a : actions()) {
        if (!a->isVisible())
            continue;
        const QRect r = actionGeometry(a);

        if (a->isSeparator()) {
            p.setPen(QPen(pal.cardStroke, bw));
            const qreal y = r.center().y() + 0.5;
            p.drawLine(QPointF(r.left() + 4, y), QPointF(r.right() - 4, y));
            continue;
        }

        const bool enabled = a->isEnabled();
        const QRectF item = QRectF(r).adjusted(0, 2, 0, -2);
        if (enabled && a == activeAction()) {
            p.setPen(Qt::NoPen);
            p.setBrush(pal.subtleFillHover);
            p.drawRoundedRect(item, pal.radiusControl, pal.radiusControl);
        }

        const QColor textColor = enabled ? pal.textPrimary : pal.textDisabled;
        const qreal cy = item.center().y();

        // icon / check column
        if (iconCol) {
            const QRect ir(r.left() + kPadLeft, qRound(cy - kIconSize / 2.0), kIconSize, kIconSize);
            if (a->isCheckable() && a->isChecked()) {
                if (a->actionGroup() && a->actionGroup()->isExclusive()) {
                    p.setPen(Qt::NoPen);
                    p.setBrush(textColor);
                    p.drawEllipse(QRectF(ir).center(), 3, 3);
                } else {
                    p.drawPixmap(ir.topLeft(), FluentIcon(Icon::Checkmark).pixmap(ir.size(), textColor, dpr));
                }
            } else if (!a->icon().isNull()) {
                a->icon().paint(&p, ir, Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);
            }
        }

        // label
        const qreal textX = r.left() + kPadLeft + (iconCol ? kIconColumn : 0);
        const bool hasSub = a->menu() != nullptr;
        const qreal rightEdge = r.right() - (hasSub ? kArrowColumn : kPadRight);
        const QString shortcut = a->shortcut().isEmpty() ? QString() : a->shortcut().toString(QKeySequence::NativeText);
        const int shortcutW = shortcut.isEmpty() ? 0 : fm.horizontalAdvance(shortcut);

        p.setFont(font());
        p.setPen(textColor);
        const qreal labelW = rightEdge - textX - (shortcutW ? shortcutW + kShortcutGap : 0);
        p.drawText(QRectF(textX, item.top(), labelW, item.height()), Qt::AlignVCenter | Qt::AlignLeft,
                   fm.elidedText(detail::plainText(a->text()), Qt::ElideRight, qRound(labelW)));
        if (shortcutW) {
            p.setPen(enabled ? pal.textSecondary : pal.textDisabled);
            p.drawText(QRectF(rightEdge - shortcutW, item.top(), shortcutW, item.height()),
                       Qt::AlignVCenter | Qt::AlignRight, shortcut);
        }

        if (hasSub) {
            const QSize as(kArrowSize, kArrowSize);
            p.drawPixmap(QPoint(r.right() - kPadRight - kArrowSize + 2, qRound(cy - kArrowSize / 2.0)),
                         FluentIcon(Icon::ChevronRight).pixmap(as, enabled ? pal.textSecondary : pal.textDisabled, dpr));
        }
    }
}

} // namespace QLight
