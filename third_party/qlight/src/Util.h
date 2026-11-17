#pragma once

// Internal helpers shared by the widgets. Not part of the public API.

#include <QColor>
#include <QRegularExpression>
#include <QString>
#include <Qt>

#include <cmath>

namespace QLight::detail {

// Strips '&' mnemonic markers ("&&" becomes a literal '&').
inline QString plainText(QString t)
{
    static const QRegularExpression single(QStringLiteral("(?<!&)&(?!&)"));
    t.remove(single);
    t.replace(QStringLiteral("&&"), QStringLiteral("&"));
    return t;
}

// Interpolate without the dark halo you get when blending from transparent black.
inline QColor lerpColor(const QColor &a, const QColor &b, qreal t)
{
    QColor from = a, to = b;
    if (a.alpha() == 0) from = QColor(b.red(), b.green(), b.blue(), 0);
    if (b.alpha() == 0) to = QColor(a.red(), a.green(), a.blue(), 0);
    return QColor::fromRgbF(from.redF() + (to.redF() - from.redF()) * t,
                            from.greenF() + (to.greenF() - from.greenF()) * t,
                            from.blueF() + (to.blueF() - from.blueF()) * t,
                            from.alphaF() + (to.alphaF() - from.alphaF()) * t);
}

// Focus ring is only shown for keyboard navigation, not mouse clicks.
inline bool isKeyboardFocusReason(Qt::FocusReason r)
{
    return r == Qt::TabFocusReason || r == Qt::BacktabFocusReason || r == Qt::ShortcutFocusReason;
}

// Hairline width snapped to whole device pixels (crisp at 125% / 150%).
inline qreal hairline(qreal dpr) { return qMax(1.0, std::round(dpr)) / dpr; }

} // namespace QLight::detail
