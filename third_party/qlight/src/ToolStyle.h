#pragma once

// Internal: a QProxyStyle that draws QToolButton (and the separators of QToolBar) the Fluent way.
// Shared by QLight::ToolButton and QLight::ToolBar. Not part of the public API.

#include "QLight/Theme.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QToolButton>

#include <cmath>

#include "QLight/FluentIcon.h"
#include "MetricStyle.h"
#include "Util.h"

namespace QLight::detail {

class ToolButtonStyle : public MetricStyle {
public:
    static constexpr int kHeight = 32;
    static constexpr int kPad = 8;
    static constexpr int kIcon = 16;
    static constexpr int kGap = 8;
    static constexpr int kMenuArrow = 20; // split-button arrow area (MenuButtonPopup)
    static constexpr int kInlineArrow = 14;

    ToolButtonStyle()
    {
        setMetric(PM_ToolBarItemSpacing, 2);
        setMetric(PM_ToolBarFrameWidth, 0);
        setMetric(PM_ToolBarItemMargin, 2);
        setMetric(PM_ToolBarHandleExtent, 0);
        setMetric(PM_ToolBarSeparatorExtent, 9);
        setMetric(PM_ToolBarExtensionExtent, 32);
        setMetric(PM_ToolBarIconSize, kIcon);
        setMetric(PM_SmallIconSize, kIcon);
        setMetric(PM_ButtonIconSize, kIcon);
    }

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
        const Palette &pal = ThemeManager::instance().palette();
        switch (pe) {
        case PE_IndicatorToolBarSeparator: {
            const qreal bw = hairline(p->device()->devicePixelRatioF());
            p->save();
            p->setPen(QPen(pal.cardStroke, bw));
            if (opt->state & State_Horizontal) // the toolbar is horizontal: a vertical hairline
                p->drawLine(QPointF(opt->rect.center().x() + 0.5, opt->rect.top() + 6),
                            QPointF(opt->rect.center().x() + 0.5, opt->rect.bottom() - 6));
            else
                p->drawLine(QPointF(opt->rect.left() + 6, opt->rect.center().y() + 0.5),
                            QPointF(opt->rect.right() - 6, opt->rect.center().y() + 0.5));
            p->restore();
            return;
        }
        case PE_IndicatorToolBarHandle:
        case PE_PanelToolBar:
        case PE_PanelButtonTool: // the button is painted as a whole in drawComplexControl
            return;
        default:
            MetricStyle::drawPrimitive(pe, opt, p, w);
        }
    }

    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
        if (ce == CE_ToolBar)
            return; // transparent: the window / card behind it shows
        MetricStyle::drawControl(ce, opt, p, w);
    }

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const override
    {
        if (cc != CC_ToolButton)
            return MetricStyle::subControlRect(cc, opt, sc, w);
        const auto *tb = qstyleoption_cast<const QStyleOptionToolButton *>(opt);
        const bool split = tb && (tb->features & QStyleOptionToolButton::MenuButtonPopup);
        QRect r = opt->rect;
        switch (sc) {
        case SC_ToolButton: return split ? r.adjusted(0, 0, -kMenuArrow, 0) : r;
        case SC_ToolButtonMenu: return split ? QRect(r.right() - kMenuArrow + 1, r.top(), kMenuArrow, r.height()) : QRect();
        default: return {};
        }
    }

    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pos,
                                     const QWidget *w) const override
    {
        if (cc != CC_ToolButton)
            return MetricStyle::hitTestComplexControl(cc, opt, pos, w);
        if (subControlRect(cc, opt, SC_ToolButtonMenu, w).contains(pos))
            return SC_ToolButtonMenu;
        return opt->rect.contains(pos) ? SC_ToolButton : SC_None;
    }

    QSize sizeFromContents(ContentsType type, const QStyleOption *opt, const QSize &csz, const QWidget *w) const override
    {
        if (type != CT_ToolButton)
            return MetricStyle::sizeFromContents(type, opt, csz, w);
        const auto *tb = qstyleoption_cast<const QStyleOptionToolButton *>(opt);
        if (!tb)
            return csz;
        const QString text = plainText(tb->text);
        const bool hasIcon = !tb->icon.isNull() || tb->arrowType != Qt::NoArrow;
        const bool hasText = !text.isEmpty() && tb->toolButtonStyle != Qt::ToolButtonIconOnly;
        const int tw = hasText ? int(std::ceil(QFontMetricsF(tb->font).horizontalAdvance(text))) + 2 : 0;
        const int is = tb->iconSize.isValid() ? tb->iconSize.width() : kIcon;

        int w2, h;
        if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon && hasIcon && hasText) {
            w2 = qMax(is, tw) + 2 * kPad;
            h = kHeight + 14;
        } else {
            w2 = 2 * kPad + (hasIcon ? is : 0) + (hasIcon && hasText ? kGap : 0) + tw;
            h = kHeight;
        }
        if (tb->features & QStyleOptionToolButton::MenuButtonPopup)
            w2 += kMenuArrow;
        else if (tb->features & QStyleOptionToolButton::HasMenu)
            w2 += kInlineArrow;
        return {qMax(w2, kHeight), h};
    }

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *w) const override
    {
        if (cc != CC_ToolButton) {
            MetricStyle::drawComplexControl(cc, opt, p, w);
            return;
        }
        const auto *tb = qstyleoption_cast<const QStyleOptionToolButton *>(opt);
        if (!tb)
            return;
        const Palette &pal = ThemeManager::instance().palette();
        const qreal dpr = p->device()->devicePixelRatioF();
        const bool enabled = tb->state & State_Enabled;
        const bool on = tb->state & State_On;
        const bool down = tb->state & State_Sunken;
        const bool hover = tb->state & State_MouseOver;
        const QRect btn = subControlRect(CC_ToolButton, tb, SC_ToolButton, w);
        const QRect menu = subControlRect(CC_ToolButton, tb, SC_ToolButtonMenu, w);

        p->save();
        p->setRenderHint(QPainter::Antialiasing);

        // background
        QColor fill = Qt::transparent;
        if (enabled) {
            if (on) fill = down ? pal.accentPressed : hover ? pal.accentHover : pal.accent;
            else if (down) fill = pal.subtleFillPressed;
            else if (hover) fill = pal.subtleFillHover;
        } else if (on) {
            fill = pal.accentDisabled;
        }
        if (fill.alpha() > 0) {
            p->setPen(Qt::NoPen);
            p->setBrush(fill);
            p->drawRoundedRect(QRectF(tb->rect), pal.radiusControl, pal.radiusControl);
        }

        const QColor fg = !enabled ? (on ? pal.textOnAccentDisabled : pal.textDisabled)
                          : on     ? pal.textOnAccent
                                   : pal.textPrimary;
        const QIcon::Mode mode = !enabled ? QIcon::Disabled : on ? QIcon::Selected : QIcon::Normal;
        const int is = tb->iconSize.isValid() ? tb->iconSize.width() : kIcon;

        // content
        QRect content = btn.adjusted(kPad, 0, -kPad, 0);
        const bool inlineArrow = (tb->features & QStyleOptionToolButton::HasMenu) && menu.isNull();
        if (inlineArrow)
            content.setRight(content.right() - kInlineArrow + 4);

        const QString text = plainText(tb->text);
        const bool hasIcon = !tb->icon.isNull() || tb->arrowType != Qt::NoArrow;
        const bool hasText = !text.isEmpty() && tb->toolButtonStyle != Qt::ToolButtonIconOnly;
        const QFontMetricsF fm(tb->font);
        const qreal tw = hasText ? fm.horizontalAdvance(text) : 0;

        auto paintIcon = [&](const QRect &r) {
            if (tb->arrowType != Qt::NoArrow) {
                const Icon ic = tb->arrowType == Qt::UpArrow ? Icon::ChevronUp
                                : tb->arrowType == Qt::DownArrow ? Icon::ChevronDown
                                : tb->arrowType == Qt::LeftArrow ? Icon::ChevronLeft
                                                                 : Icon::ChevronRight;
                p->drawPixmap(r.topLeft(), FluentIcon(ic).pixmap(r.size(), fg, dpr));
            } else {
                tb->icon.paint(p, r, Qt::AlignCenter, mode, on ? QIcon::On : QIcon::Off);
            }
        };

        p->setFont(tb->font);
        p->setPen(fg);
        if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon && hasIcon && hasText) {
            const int total = is + 4 + int(fm.height());
            const int top = content.center().y() - total / 2;
            paintIcon(QRect(content.center().x() - is / 2, top, is, is));
            p->drawText(QRectF(content.left(), top + is + 4, content.width(), fm.height()), Qt::AlignHCenter | Qt::AlignTop,
                        fm.elidedText(text, Qt::ElideRight, content.width()));
        } else {
            const qreal group = (hasIcon ? is : 0) + (hasIcon && hasText ? kGap : 0) + tw;
            qreal x = content.center().x() - group / 2.0;
            if (hasIcon) {
                paintIcon(QRect(qRound(x), content.center().y() - is / 2, is, is));
                x += is + (hasText ? kGap : 0);
            }
            if (hasText)
                p->drawText(QRectF(x, content.top(), tw + 2, content.height()), Qt::AlignVCenter | Qt::AlignLeft, text);
        }

        // drop-down chevron: inline (whole button opens the menu) or split (own area)
        if (tb->features & QStyleOptionToolButton::HasMenu || !menu.isNull()) {
            const QRect area = menu.isNull() ? QRect(btn.right() - kInlineArrow - 2, btn.top(), kInlineArrow, btn.height()) : menu;
            if (!menu.isNull() && (hover || down)) { // hairline between the two halves of a split button
                p->setPen(QPen(pal.cardStroke, hairline(dpr)));
                p->drawLine(QPointF(area.left() + 0.5, area.top() + 7), QPointF(area.left() + 0.5, area.bottom() - 7));
            }
            p->drawPixmap(area.center() - QPoint(5, 5), FluentIcon(Icon::ChevronDown).pixmap(10, fg, dpr));
        }

        // keyboard focus ring
        if ((tb->state & State_HasFocus) && (tb->state & State_KeyboardFocusChange) && enabled) {
            p->setBrush(Qt::NoBrush);
            p->setPen(QPen(pal.focusStroke, 2));
            p->drawRoundedRect(QRectF(tb->rect).adjusted(1, 1, -1, -1), pal.radiusControl + 1, pal.radiusControl + 1);
        }
        p->restore();
    }
};

} // namespace QLight::detail
