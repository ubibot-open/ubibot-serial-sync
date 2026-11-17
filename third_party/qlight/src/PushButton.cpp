#include "QLight/PushButton.h"

#include "QLight/Theme.h"

#include <QEasingCurve>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QPainter>

#include "Util.h"

namespace QLight {

namespace {

constexpr int kHeight = 32;
constexpr int kPaddingX = 16;
constexpr int kMinWidth = 64;
constexpr int kIconSize = 16;
constexpr int kIconGap = 8;
constexpr int kAnimMs = 120;

} // namespace

PushButton::PushButton(QWidget *parent) : QPushButton(parent) { init(); }
PushButton::PushButton(const QString &text, QWidget *parent) : QPushButton(text, parent) { init(); }
PushButton::PushButton(const QString &text, Variant variant, QWidget *parent)
    : QPushButton(text, parent), m_variant(variant)
{
    init();
}

void PushButton::init()
{
    setFocusPolicy(Qt::StrongFocus);
    m_anim.setDuration(kAnimMs);
    m_anim.setEasingCurve(QEasingCurve::OutCubic);
    m_anim.setStartValue(0.0);
    m_anim.setEndValue(1.0);
    connect(&m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        const qreal t = v.toReal();
        m_fill = detail::lerpColor(m_fromFill, m_toFill, t);
        m_text = detail::lerpColor(m_fromText, m_toText, t);
        update();
    });

    connect(this, &QAbstractButton::pressed, this, [this] { syncTarget(true); });
    connect(this, &QAbstractButton::released, this, [this] { syncTarget(true); });
    connect(this, &QAbstractButton::toggled, this, [this] { syncTarget(true); });
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            [this] { syncTarget(false); });

    syncTarget(false);
}

void PushButton::setVariant(Variant v)
{
    if (m_variant == v)
        return;
    m_variant = v;
    syncTarget(false);
    updateGeometry();
}

void PushButton::setFluentIcon(const FluentIcon &icon)
{
    m_fluentIcon = icon;
    updateAutoAccessibleName();
    updateGeometry();
    update();
}

// An icon-only button has no text for a screen reader; use its tooltip as the accessible name
// unless the application set one explicitly.
void PushButton::updateAutoAccessibleName()
{
    if (!accessibleName().isEmpty() && !m_autoAccessibleName)
        return;
    if (text().isEmpty() && !toolTip().isEmpty()) {
        setAccessibleName(toolTip());
        m_autoAccessibleName = true;
    } else if (m_autoAccessibleName) {
        setAccessibleName(QString());
        m_autoAccessibleName = false;
    }
}

bool PushButton::accentLook() const
{
    return m_variant == Accent || (isCheckable() && isChecked());
}

QColor PushButton::targetFill() const
{
    const Palette &p = ThemeManager::instance().palette();
    const bool hover = underMouse();
    const bool down = isDown();
    if (accentLook()) {
        if (!isEnabled()) return p.accentDisabled;
        return down ? p.accentPressed : hover ? p.accentHover : p.accent;
    }
    if (m_variant == Subtle) {
        if (!isEnabled()) return Qt::transparent;
        return down ? p.subtleFillPressed : hover ? p.subtleFillHover : QColor(Qt::transparent);
    }
    if (!isEnabled()) return p.controlFillDisabled;
    return down ? p.controlFillPressed : hover ? p.controlFillHover : p.controlFill;
}

QColor PushButton::targetText() const
{
    const Palette &p = ThemeManager::instance().palette();
    if (accentLook()) {
        if (!isEnabled()) return p.textOnAccentDisabled;
        QColor c = p.textOnAccent;
        if (isDown()) c.setAlpha(200);
        return c;
    }
    if (!isEnabled()) return p.textDisabled;
    return isDown() ? p.textSecondary : p.textPrimary;
}

void PushButton::syncTarget(bool animate)
{
    m_anim.stop();
    m_toFill = targetFill();
    m_toText = targetText();
    if (!animate) {
        m_fill = m_toFill;
        m_text = m_toText;
        update();
        return;
    }
    m_fromFill = m_fill;
    m_fromText = m_text;
    m_anim.start();
}

QSize PushButton::sizeHint() const
{
    const QFontMetrics fm(font());
    int w = fm.horizontalAdvance(detail::plainText(text()));
    if (!icon().isNull() || !m_fluentIcon.isNull())
        w += kIconSize + (text().isEmpty() ? 0 : kIconGap);
    return {qMax(kMinWidth, w + 2 * kPaddingX), kHeight};
}

bool PushButton::event(QEvent *e)
{
    const bool handled = QPushButton::event(e);
    if (e->type() == QEvent::ToolTipChange) // not routed to changeEvent()
        updateAutoAccessibleName();
    return handled;
}

void PushButton::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Snap the hairline border to a whole number of device pixels so it stays
    // crisp at fractional scale factors (125% / 150%).
    const qreal dpr = devicePixelRatioF();
    const qreal bw = detail::hairline(dpr);
    const QRectF r = QRectF(rect()).adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2);
    const qreal rad = pal.radiusControl;

    // body
    p.setPen(Qt::NoPen);
    p.setBrush(m_fill);
    p.drawRoundedRect(r, rad, rad);

    // border: light top edge, slightly heavier bottom edge (Fluent "elevation" hint)
    if (m_variant != Subtle || accentLook()) {
        QColor top, bottom;
        if (accentLook()) {
            top = pal.accentStrokeTop;
            bottom = pal.accentStrokeBottom;
        } else {
            top = pal.controlStrokeTop;
            bottom = pal.controlStrokeBottom;
        }
        if (!isEnabled()) bottom = top;
        QLinearGradient g(r.topLeft(), r.bottomLeft());
        const qreal edge = 1.0 - 3.0 / qMax(1, height());
        g.setColorAt(0, top);
        g.setColorAt(edge, top);
        g.setColorAt(1, bottom);
        p.setPen(QPen(QBrush(g), bw));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r, rad, rad);
    }

    // content (icon + text, centered)
    const QString label = detail::plainText(text());
    const QFontMetrics fm(font());
    const int textW = fm.horizontalAdvance(label);
    const bool hasIcon = !icon().isNull() || !m_fluentIcon.isNull();
    const int total = (hasIcon ? kIconSize : 0) + (hasIcon && !label.isEmpty() ? kIconGap : 0) + textW;
    qreal x = (width() - total) / 2.0;

    if (hasIcon) {
        const QRect ir(qRound(x), (height() - kIconSize) / 2, kIconSize, kIconSize);
        if (!m_fluentIcon.isNull()) {
            p.drawPixmap(ir.topLeft(), m_fluentIcon.pixmap(ir.size(), m_text, devicePixelRatioF()));
        } else {
            const QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
            icon().paint(&p, ir, Qt::AlignCenter, mode, isChecked() ? QIcon::On : QIcon::Off);
        }
        x += kIconSize + kIconGap;
    }
    p.setPen(m_text);
    p.setFont(font());
    p.drawText(QRectF(x, 0, textW + 1, height()), Qt::AlignVCenter | Qt::AlignLeft, label);

    // keyboard focus ring: 2px outer stroke + 1px inner gap in surface color
    if (m_focusVisible && isEnabled()) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(pal.focusStroke, 2));
        p.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), rad + 1, rad + 1);
        p.setPen(QPen(pal.windowBg, 1));
        p.drawRoundedRect(QRectF(rect()).adjusted(2.5, 2.5, -2.5, -2.5), rad, rad);
    }
}

void PushButton::enterEvent(QEnterEvent *e)
{
    QPushButton::enterEvent(e);
    syncTarget(true);
}

void PushButton::leaveEvent(QEvent *e)
{
    QPushButton::leaveEvent(e);
    syncTarget(true);
}

void PushButton::focusInEvent(QFocusEvent *e)
{
    QPushButton::focusInEvent(e);
    m_focusVisible = detail::isKeyboardFocusReason(e->reason());
    update();
}

void PushButton::focusOutEvent(QFocusEvent *e)
{
    QPushButton::focusOutEvent(e);
    m_focusVisible = false;
    update();
}

void PushButton::changeEvent(QEvent *e)
{
    QPushButton::changeEvent(e);
    if (e->type() == QEvent::EnabledChange)
        syncTarget(false);
    else if (e->type() == QEvent::FontChange)
        updateGeometry();

}

} // namespace QLight
