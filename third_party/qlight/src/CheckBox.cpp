#include "QLight/CheckBox.h"

#include "QLight/Theme.h"

#include <QFocusEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

#include "ColorAnimator.h"
#include "Util.h"

namespace QLight {

namespace {
constexpr int kBox = 20;
constexpr int kInset = 2; // room for the focus ring
constexpr int kGap = 8;
constexpr int kHeight = 32;
} // namespace

CheckBox::CheckBox(QWidget *parent) : QCheckBox(parent) { init(); }
CheckBox::CheckBox(const QString &text, QWidget *parent) : QCheckBox(text, parent) { init(); }

void CheckBox::init()
{
    m_fill = new detail::ColorAnimator(this);
    m_stroke = new detail::ColorAnimator(this);
    m_mark = new detail::ColorAnimator(this);
    for (auto *a : {m_fill, m_stroke, m_mark})
        connect(a, &detail::ColorAnimator::updated, this, qOverload<>(&QWidget::update));

    setFocusPolicy(Qt::StrongFocus);
    connect(this, &QAbstractButton::pressed, this, [this] { syncTarget(true); });
    connect(this, &QAbstractButton::released, this, [this] { syncTarget(true); });
    connect(this, &QCheckBox::checkStateChanged, this, [this] { syncTarget(true); }); // Qt 6.7+
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            [this] { syncTarget(false); });
    syncTarget(false);
}

void CheckBox::syncTarget(bool animate)
{
    const Palette &p = ThemeManager::instance().palette();
    const bool on = checkState() != Qt::Unchecked;
    const bool hover = underMouse(), down = isDown(), enabled = isEnabled();

    QColor fill, stroke, mark;
    if (on) {
        fill = !enabled ? p.accentDisabled
               : down   ? p.accentPressed
               : hover  ? p.accentHover
                        : p.accent;
        stroke = Qt::transparent;
        mark = enabled ? p.textOnAccent : p.textOnAccentDisabled;
        if (enabled && down) mark.setAlpha(200);
    } else {
        fill = !enabled ? Qt::transparent
               : down   ? p.controlFillPressed
               : hover  ? p.controlFillHover
                        : p.controlFill;
        stroke = enabled ? p.controlStrokeStrong : p.textDisabled;
        mark = Qt::transparent;
    }
    m_fill->setTarget(fill, animate);
    m_stroke->setTarget(stroke, animate);
    m_mark->setTarget(mark, animate);
}

QSize CheckBox::sizeHint() const
{
    const QString label = detail::plainText(text());
    int w = 2 * kInset + kBox;
    if (!label.isEmpty())
        w += kGap + QFontMetrics(font()).horizontalAdvance(label);
    return {w, label.isEmpty() ? kBox + 2 * kInset : kHeight};
}

bool CheckBox::hitButton(const QPoint &pos) const { return rect().contains(pos); }

void CheckBox::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal bw = detail::hairline(devicePixelRatioF());
    const QRectF box(kInset, (height() - kBox) / 2.0, kBox, kBox);
    const qreal rad = pal.radiusControl;

    p.setPen(Qt::NoPen);
    p.setBrush(m_fill->color());
    p.drawRoundedRect(box.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), rad, rad);
    if (m_stroke->color().alpha() > 0) {
        p.setPen(QPen(m_stroke->color(), bw));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(box.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), rad, rad);
    }

    // mark: check for Checked, dash for PartiallyChecked
    if (m_mark->color().alpha() > 0) {
        QPen pen(m_mark->color(), 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.save();
        p.translate(box.topLeft());
        if (checkState() == Qt::PartiallyChecked) {
            p.drawLine(QPointF(5.5, 10), QPointF(14.5, 10));
        } else {
            QPainterPath path;
            path.moveTo(5.2, 10.4);
            path.lineTo(8.6, 13.6);
            path.lineTo(14.8, 6.6);
            p.drawPath(path);
        }
        p.restore();
    }

    // label
    const QString label = detail::plainText(text());
    if (!label.isEmpty()) {
        p.setPen(isEnabled() ? pal.textPrimary : pal.textDisabled);
        p.setFont(font());
        const qreal x = kInset + kBox + kGap;
        p.drawText(QRectF(x, 0, width() - x, height()), Qt::AlignVCenter | Qt::AlignLeft, label);
    }

    if (m_focusVisible && isEnabled()) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(pal.focusStroke, 2));
        p.drawRoundedRect(box.adjusted(-1, -1, 1, 1), rad + 1, rad + 1);
    }
}

void CheckBox::enterEvent(QEnterEvent *e)
{
    QCheckBox::enterEvent(e);
    syncTarget(true);
}

void CheckBox::leaveEvent(QEvent *e)
{
    QCheckBox::leaveEvent(e);
    syncTarget(true);
}

void CheckBox::focusInEvent(QFocusEvent *e)
{
    QCheckBox::focusInEvent(e);
    m_focusVisible = detail::isKeyboardFocusReason(e->reason());
    update();
}

void CheckBox::focusOutEvent(QFocusEvent *e)
{
    QCheckBox::focusOutEvent(e);
    m_focusVisible = false;
    update();
}

void CheckBox::changeEvent(QEvent *e)
{
    QCheckBox::changeEvent(e);
    if (e->type() == QEvent::EnabledChange)
        syncTarget(false);
    else if (e->type() == QEvent::FontChange)
        updateGeometry();
}

} // namespace QLight
