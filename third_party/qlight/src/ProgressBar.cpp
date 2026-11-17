#include "QLight/ProgressBar.h"

#include "QLight/Theme.h"

#include <QEasingCurve>
#include <QEvent>
#include <QPainter>

#include <cmath>

namespace QLight {

namespace {
constexpr int kThickness = 4;
constexpr int kLongSide = 150;
constexpr int kValueMs = 200;
constexpr int kBusyMs = 1600;
} // namespace

ProgressBar::ProgressBar(QWidget *parent) : QProgressBar(parent)
{
    setTextVisible(false);
    setSizePolicy(orientation() == Qt::Horizontal ? QSizePolicy::Expanding : QSizePolicy::Fixed,
                  orientation() == Qt::Horizontal ? QSizePolicy::Fixed : QSizePolicy::Expanding);

    m_valueAnim.setDuration(kValueMs);
    m_valueAnim.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_valueAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_fraction = v.toReal();
        update();
    });

    m_busyAnim.setDuration(kBusyMs);
    m_busyAnim.setStartValue(0.0);
    m_busyAnim.setEndValue(1.0);
    m_busyAnim.setLoopCount(-1);
    connect(&m_busyAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_busyPhase = v.toReal();
        update();
    });

    connect(this, &QProgressBar::valueChanged, this, [this] { syncValue(isVisible()); });
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            qOverload<>(&QWidget::update));
    syncValue(false);
}

void ProgressBar::setState(State s)
{
    if (m_state == s)
        return;
    m_state = s;
    update();
}

QSize ProgressBar::sizeHint() const
{
    return orientation() == Qt::Horizontal ? QSize(kLongSide, kThickness) : QSize(kThickness, kLongSide);
}

QSize ProgressBar::minimumSizeHint() const
{
    return orientation() == Qt::Horizontal ? QSize(16, kThickness) : QSize(kThickness, 16);
}

qreal ProgressBar::targetFraction() const
{
    const int range = maximum() - minimum();
    if (range <= 0 || value() < minimum())
        return 0;
    return qBound(0.0, qreal(value() - minimum()) / range, 1.0);
}

void ProgressBar::syncValue(bool animate)
{
    m_valueAnim.stop();
    const qreal target = targetFraction();
    if (!animate) {
        m_fraction = target;
        update();
        return;
    }
    m_valueAnim.setStartValue(m_fraction);
    m_valueAnim.setEndValue(target);
    m_valueAnim.start();
}

void ProgressBar::syncBusy()
{
    if (busy() && isVisible() && isEnabled()) {
        if (m_busyAnim.state() != QAbstractAnimation::Running)
            m_busyAnim.start();
    } else {
        m_busyAnim.stop();
    }
    update();
}

void ProgressBar::showEvent(QShowEvent *e)
{
    QProgressBar::showEvent(e);
    syncValue(false);
    syncBusy();
}

void ProgressBar::hideEvent(QHideEvent *e)
{
    QProgressBar::hideEvent(e);
    syncBusy();
}

void ProgressBar::changeEvent(QEvent *e)
{
    QProgressBar::changeEvent(e);
    if (e->type() == QEvent::EnabledChange)
        syncBusy();
}

void ProgressBar::paintEvent(QPaintEvent *)
{
    // QProgressBar has no range-changed signal (setRange is not virtual), so notice
    // range switches, e.g. (0, 0) <-> determinate, when the repaint arrives.
    if (m_lastMin != minimum() || m_lastMax != maximum()) {
        m_lastMin = minimum();
        m_lastMax = maximum();
        syncValue(false);
        syncBusy();
    }
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const bool horizontal = orientation() == Qt::Horizontal;
    const qreal len = horizontal ? width() : height();
    const qreal thick = horizontal ? height() : width();

    // paint in a horizontal frame: vertical bars fill from the bottom up
    if (!horizontal) {
        p.translate(0, height());
        p.rotate(-90);
    }
    const bool flip = horizontal && (invertedAppearance() != (layoutDirection() == Qt::RightToLeft));
    if (flip) {
        p.translate(len, 0);
        p.scale(-1, 1);
    }

    const qreal rad = thick / 2.0;
    p.setPen(Qt::NoPen);
    p.setBrush(isEnabled() ? pal.trackFill : pal.textDisabled);
    p.drawRoundedRect(QRectF(0, 0, len, thick), rad, rad);

    QColor bar = m_state == Paused ? pal.caution : m_state == Error ? pal.critical : pal.accent;
    if (!isEnabled())
        bar = pal.accentDisabled;
    p.setBrush(bar);

    if (busy()) {
        // two bars sweeping across, the second trailing the first
        struct Seg { qreal phase; qreal length; };
        for (const Seg &s : {Seg{m_busyPhase, 0.35}, Seg{m_busyPhase + 0.45, 0.2}}) {
            const qreal ph = s.phase - std::floor(s.phase);
            const qreal e = QEasingCurve(QEasingCurve::InOutSine).valueForProgress(ph);
            const qreal segLen = len * s.length;
            const qreal x = -segLen + (len + segLen) * e;
            const qreal x0 = qMax(0.0, x), x1 = qMin(len, x + segLen);
            if (x1 > x0)
                p.drawRoundedRect(QRectF(x0, 0, x1 - x0, thick), rad, rad);
        }
        return;
    }
    if (m_fraction > 0) {
        const qreal w = qMax(thick, len * m_fraction); // keep the cap round for tiny values
        p.drawRoundedRect(QRectF(0, 0, w, thick), rad, rad);
    }
}

} // namespace QLight
