#pragma once

// Internal: where to put a popup relative to a target rectangle. Not part of the public API.

#include <QRect>
#include <QSize>

namespace QLight::detail {

enum class Side { Bottom, Top, Right, Left }; // the side of the target the popup sits on

struct Placed {
    QPoint topLeft;
    Side side = Side::Bottom;
};

// `preferred` < 0 means "automatic": bottom, top, right, left - the first one that fits, else
// the one with the most room. Along the other axis the popup is centred on the target and then
// kept inside `avail`. `gap` is the distance between target and popup.
inline Placed placeAround(const QRect &target, const QSize &size, int preferred, int gap, const QRect &avail)
{
    auto rectFor = [&](Side s) {
        QPoint p;
        switch (s) {
        case Side::Bottom: p = QPoint(target.center().x() - size.width() / 2, target.bottom() + 1 + gap); break;
        case Side::Top: p = QPoint(target.center().x() - size.width() / 2, target.top() - gap - size.height()); break;
        case Side::Right: p = QPoint(target.right() + 1 + gap, target.center().y() - size.height() / 2); break;
        case Side::Left: p = QPoint(target.left() - gap - size.width(), target.center().y() - size.height() / 2); break;
        }
        return QRect(p, size);
    };
    auto fits = [&](Side s) {
        const QRect r = rectFor(s);
        // only the axis the popup is stacked along has to fit; the other axis gets clamped
        return (s == Side::Bottom || s == Side::Top) ? (r.top() >= avail.top() && r.bottom() <= avail.bottom())
                                                     : (r.left() >= avail.left() && r.right() <= avail.right());
    };
    auto room = [&](Side s) {
        switch (s) {
        case Side::Bottom: return avail.bottom() - target.bottom();
        case Side::Top: return target.top() - avail.top();
        case Side::Right: return avail.right() - target.right();
        default: return target.left() - avail.left();
        }
    };

    Side order[4] = {Side::Bottom, Side::Top, Side::Right, Side::Left};
    Side chosen = Side::Bottom;
    if (preferred >= 0) {
        chosen = Side(preferred);
        if (!fits(chosen)) { // flip to the opposite side when that one fits
            const Side opposite = chosen == Side::Bottom ? Side::Top
                                  : chosen == Side::Top    ? Side::Bottom
                                  : chosen == Side::Right  ? Side::Left
                                                           : Side::Right;
            if (fits(opposite))
                chosen = opposite;
        }
    } else {
        bool found = false;
        for (Side s : order)
            if (fits(s)) {
                chosen = s;
                found = true;
                break;
            }
        if (!found) {
            chosen = order[0];
            for (Side s : order)
                if (room(s) > room(chosen))
                    chosen = s;
        }
    }

    QRect r = rectFor(chosen);
    const int x = qBound(avail.left(), r.left(), qMax(avail.left(), avail.right() - r.width() + 1));
    const int y = qBound(avail.top(), r.top(), qMax(avail.top(), avail.bottom() - r.height() + 1));
    return {QPoint(x, y), chosen};
}

} // namespace QLight::detail
