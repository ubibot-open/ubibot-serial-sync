#pragma once

#include <QColor>
#include <QRectF>

#include "QLight/Export.h"

class QPainter;

namespace QLight {

// Soft drop shadow for rounded rectangles.
//
// The blurred shadow is rendered once into a small 9-patch pixmap (DPR-aware)
// and then stretched to any size, so painting costs nine pixmap blits and no
// per-frame blur (unlike QGraphicsDropShadowEffect).
struct ShadowSpec {
    qreal radius = 8;    // corner radius of the casting rect
    qreal blur = 10;     // blur extent in logical px (shadow spreads this far)
    qreal offsetY = 2;   // vertical offset in logical px
    QColor color{0, 0, 0, 40};

    bool operator==(const ShadowSpec &o) const
    {
        return radius == o.radius && blur == o.blur && offsetY == o.offsetY && color == o.color;
    }
};

namespace Shadow {

// Space the shadow needs around the casting rect on each side (logical px).
QLIGHT_EXPORT qreal extent(const ShadowSpec &spec);

// Paints the shadow of `rect` (the casting rect, logical coords) onto `p`.
QLIGHT_EXPORT void draw(QPainter *p, const QRectF &rect, const ShadowSpec &spec, qreal dpr);

} // namespace Shadow
} // namespace QLight
