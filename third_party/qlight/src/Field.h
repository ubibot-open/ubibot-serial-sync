#pragma once

// Internal: shared look of text-entry "fields" (LineEdit, SpinBox, TextEdit, ...).
// Not part of the public API.

#include "QLight/Theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QWidget>

#include "Util.h"

namespace QLight::detail {

// Fill colour of an entry field for its current state.
inline QColor fieldFill(bool enabled, bool focused, bool hovered)
{
    const Palette &p = ThemeManager::instance().palette();
    if (!enabled) return p.controlFillDisabled;
    if (focused) return p.inputFillFocused;
    if (hovered) return p.controlFillHover;
    return p.controlFill;
}

// Rounded body, 1px border with a stronger bottom edge, or an accent underline while focused.
inline void paintFieldFrame(QPainter &p, const QRectF &widgetRect, qreal dpr, const QColor &fill, bool focused,
                            bool enabled)
{
    const Palette &pal = ThemeManager::instance().palette();
    p.save();
    p.setRenderHint(QPainter::Antialiasing);

    const qreal bw = hairline(dpr);
    const QRectF r = widgetRect.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2);
    const qreal rad = pal.radiusControl;

    p.setPen(Qt::NoPen);
    p.setBrush(fill);
    p.drawRoundedRect(r, rad, rad);

    const bool accent = focused && enabled;
    const QColor top = pal.controlStrokeTop;
    const QColor bottom = enabled ? pal.controlStrokeStrong : top;
    QLinearGradient g(r.topLeft(), r.bottomLeft());
    const qreal edge = 1.0 - 4.0 / qMax(1.0, widgetRect.height());
    g.setColorAt(0, top);
    g.setColorAt(edge, top);
    g.setColorAt(1, accent ? top : bottom);
    p.setPen(QPen(QBrush(g), bw));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, rad, rad);

    if (accent) {
        QPainterPath clip;
        clip.addRoundedRect(r, rad, rad);
        p.setClipPath(clip);
        p.setPen(Qt::NoPen);
        p.setBrush(pal.accent);
        p.drawRect(QRectF(widgetRect.left(), widgetRect.bottom() - 2.0, widgetRect.width(), 2.0));
    }
    p.restore();
}

// Text / placeholder / selection colours for a widget that draws its own background.
inline void applyFieldPalette(QWidget *w)
{
    const Palette &p = ThemeManager::instance().palette();
    QPalette pal = w->palette();
    for (auto group : {QPalette::Active, QPalette::Inactive}) {
        pal.setColor(group, QPalette::Base, Qt::transparent);
        pal.setColor(group, QPalette::Text, p.textPrimary);
        pal.setColor(group, QPalette::WindowText, p.textPrimary);
        pal.setColor(group, QPalette::PlaceholderText, p.textPlaceholder);
        pal.setColor(group, QPalette::Highlight, p.accent);
        pal.setColor(group, QPalette::HighlightedText, p.textOnAccent);
    }
    pal.setColor(QPalette::Disabled, QPalette::Base, Qt::transparent);
    pal.setColor(QPalette::Disabled, QPalette::Text, p.textDisabled);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, p.textDisabled);
    pal.setColor(QPalette::Disabled, QPalette::PlaceholderText, p.textDisabled);
    w->setPalette(pal);
}

} // namespace QLight::detail
