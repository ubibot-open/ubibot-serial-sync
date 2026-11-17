#include "QLight/LineEdit.h"

#include "QLight/Theme.h"

#include <QFocusEvent>
#include <QPainter>
#include <QPainterPath>

#include "ColorAnimator.h"
#include "Field.h"
#include "Util.h"

namespace QLight {

namespace {
constexpr int kHeight = 32;
constexpr int kMinWidth = 120;
constexpr int kPaddingX = 10;
} // namespace

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent), m_fill(new detail::ColorAnimator(this)) { init(); }
LineEdit::LineEdit(const QString &text, QWidget *parent)
    : QLineEdit(text, parent), m_fill(new detail::ColorAnimator(this))
{
    init();
}

void LineEdit::init()
{
    // The frame and base fill are painted by us; QLineEdit only draws text/caret.
    setFrame(false);
    setTextMargins(kPaddingX - 2, 0, kPaddingX - 2, 0);

    connect(m_fill, &detail::ColorAnimator::updated, this, qOverload<>(&QWidget::update));
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] {
        applyPalette();
        syncTarget(false);
    });
    applyPalette();
    syncTarget(false);
}

void LineEdit::applyPalette() { detail::applyFieldPalette(this); }

void LineEdit::syncTarget(bool animate)
{
    m_fill->setTarget(detail::fieldFill(isEnabled(), hasFocus(), underMouse()), animate);
}

QSize LineEdit::sizeHint() const
{
    return {qMax(kMinWidth, QLineEdit::sizeHint().width()), kHeight};
}

QSize LineEdit::minimumSizeHint() const { return {kMinWidth, kHeight}; }

void LineEdit::paintEvent(QPaintEvent *e)
{
    {
        QPainter p(this);
        detail::paintFieldFrame(p, QRectF(rect()), devicePixelRatioF(), m_fill->color(), hasFocus(), isEnabled());
    }
    QLineEdit::paintEvent(e);
}

void LineEdit::enterEvent(QEnterEvent *e)
{
    QLineEdit::enterEvent(e);
    syncTarget(true);
}

void LineEdit::leaveEvent(QEvent *e)
{
    QLineEdit::leaveEvent(e);
    syncTarget(true);
}

void LineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    syncTarget(true);
    update();
}

void LineEdit::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    syncTarget(true);
    update();
}

void LineEdit::changeEvent(QEvent *e)
{
    QLineEdit::changeEvent(e);
    if (e->type() == QEvent::EnabledChange)
        syncTarget(false);
    else if (e->type() == QEvent::FontChange)
        updateGeometry();
}

} // namespace QLight
