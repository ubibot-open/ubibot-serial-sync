#include "QLight/RadioButton.h"

#include "QLight/Theme.h"

#include <QFocusEvent>
#include <QFontMetrics>
#include <QPainter>

#include "ColorAnimator.h"
#include "Util.h"

namespace QLight {

namespace {
constexpr int kCircle = 20;
constexpr int kInset = 2; // room for the focus ring
constexpr int kGap = 8;
constexpr int kHeight = 32;
constexpr qreal kDotRest = 10;
constexpr qreal kDotHover = 12;
constexpr qreal kDotPressed = 8;
constexpr int kAnimMs = 120;
} // namespace

RadioButton::RadioButton(QWidget *parent) : QRadioButton(parent) { init(); }
RadioButton::RadioButton(const QString &text, QWidget *parent) : QRadioButton(text, parent) { init(); }

void RadioButton::init()
{
    m_fill = new detail::ColorAnimator(this);
    m_stroke = new detail::ColorAnimator(this);
    m_dotColor = new detail::ColorAnimator(this);
    for (auto *a : {m_fill, m_stroke, m_dotColor})
        connect(a, &detail::ColorAnimator::updated, this, qOverload<>(&QWidget::update));

    m_dotAnim.setDuration(kAnimMs);
    m_dotAnim.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_dotAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_dot = v.toReal();
        update();
    });

    setFocusPolicy(Qt::StrongFocus);
    connect(this, &QAbstractButton::pressed, this, [this] { syncTarget(true); });
    connect(this, &QAbstractButton::released, this, [this] { syncTarget(true); });
    connect(this, &QAbstractButton::toggled, this, [this] { syncTarget(true); });
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            [this] { syncTarget(false); });
    syncTarget(false);
}

void RadioButton::syncTarget(bool animate)
{
    const Palette &p = ThemeManager::instance().palette();
    const bool on = isChecked();
    const bool hover = underMouse(), down = isDown(), enabled = isEnabled();

    QColor fill, stroke, dot;
    if (on) {
        fill = !enabled ? p.accentDisabled : down ? p.accentPressed : hover ? p.accentHover : p.accent;
        stroke = Qt::transparent;
        dot = enabled ? p.textOnAccent : p.textOnAccentDisabled;
    } else {
        fill = !enabled ? Qt::transparent
               : down   ? p.controlFillPressed
               : hover  ? p.controlFillHover
                        : p.controlFill;
        stroke = enabled ? p.controlStrokeStrong : p.textDisabled;
        dot = Qt::transparent;
    }
    m_fill->setTarget(fill, animate);
    m_stroke->setTarget(stroke, animate);
    m_dotColor->setTarget(dot, animate);

    const qreal target = !on ? 0 : (!enabled ? kDotRest : down ? kDotPressed : hover ? kDotHover : kDotRest);
    m_dotAnim.stop();
    if (!animate) {
        m_dot = target;
        update();
        return;
    }
    m_dotAnim.setStartValue(m_dot);
    m_dotAnim.setEndValue(target);
    m_dotAnim.start();
}

QSize RadioButton::sizeHint() const
{
    const QString label = detail::plainText(text());
    int w = 2 * kInset + kCircle;
    if (!label.isEmpty())
        w += kGap + QFontMetrics(font()).horizontalAdvance(label);
    return {w, label.isEmpty() ? kCircle + 2 * kInset : kHeight};
}

bool RadioButton::hitButton(const QPoint &pos) const { return rect().contains(pos); }

void RadioButton::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal bw = detail::hairline(devicePixelRatioF());
    const QRectF circle(kInset, (height() - kCircle) / 2.0, kCircle, kCircle);
    const QRectF body = circle.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2);

    p.setPen(Qt::NoPen);
    p.setBrush(m_fill->color());
    p.drawEllipse(body);
    if (m_stroke->color().alpha() > 0) {
        p.setPen(QPen(m_stroke->color(), bw));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(body);
    }
    if (m_dot > 0.1 && m_dotColor->color().alpha() > 0) {
        p.setPen(Qt::NoPen);
        p.setBrush(m_dotColor->color());
        p.drawEllipse(circle.center(), m_dot / 2, m_dot / 2);
    }

    const QString label = detail::plainText(text());
    if (!label.isEmpty()) {
        p.setPen(isEnabled() ? pal.textPrimary : pal.textDisabled);
        p.setFont(font());
        const qreal x = kInset + kCircle + kGap;
        p.drawText(QRectF(x, 0, width() - x, height()), Qt::AlignVCenter | Qt::AlignLeft, label);
    }

    if (m_focusVisible && isEnabled()) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(pal.focusStroke, 2));
        p.drawEllipse(circle.center(), kCircle / 2.0 + 2, kCircle / 2.0 + 2);
    }
}

void RadioButton::enterEvent(QEnterEvent *e)
{
    QRadioButton::enterEvent(e);
    syncTarget(true);
}

void RadioButton::leaveEvent(QEvent *e)
{
    QRadioButton::leaveEvent(e);
    syncTarget(true);
}

void RadioButton::focusInEvent(QFocusEvent *e)
{
    QRadioButton::focusInEvent(e);
    m_focusVisible = detail::isKeyboardFocusReason(e->reason());
    update();
}

void RadioButton::focusOutEvent(QFocusEvent *e)
{
    QRadioButton::focusOutEvent(e);
    m_focusVisible = false;
    update();
}

void RadioButton::changeEvent(QEvent *e)
{
    QRadioButton::changeEvent(e);
    if (e->type() == QEvent::EnabledChange)
        syncTarget(false);
    else if (e->type() == QEvent::FontChange)
        updateGeometry();
}

} // namespace QLight
