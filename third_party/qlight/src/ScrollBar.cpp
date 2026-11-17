#include "QLight/ScrollBar.h"

#include "QLight/Theme.h"

#include <QAbstractScrollArea>
#include <QMouseEvent>
#include <QPainter>

#include "ColorAnimator.h"

namespace QLight {

namespace {
constexpr int kExtent = 12;        // bar thickness (cross axis)
constexpr qreal kRestThick = 2;    // handle thickness at rest
constexpr qreal kActiveThick = 6;  // handle thickness on hover / drag
constexpr qreal kEndMargin = 2;    // gap at both ends of the groove
constexpr qreal kMinHandle = 24;
constexpr int kAnimMs = 150;
} // namespace

ScrollBar::ScrollBar(Qt::Orientation o, QWidget *parent)
    : QScrollBar(o, parent), m_color(new detail::ColorAnimator(this, kAnimMs))
{
    init();
}

ScrollBar::ScrollBar(QWidget *parent) : ScrollBar(Qt::Vertical, parent) {}

void ScrollBar::replaceIn(QAbstractScrollArea *area)
{
    area->setVerticalScrollBar(new ScrollBar(Qt::Vertical, area));
    area->setHorizontalScrollBar(new ScrollBar(Qt::Horizontal, area));
}

namespace {

// Mirrors the (hidden) native scroll bars of an area onto ScrollBar handles floating over the viewport.
class OverlayBars : public QObject {
public:
    explicit OverlayBars(QAbstractScrollArea *area) : QObject(area), m_area(area)
    {
        setObjectName(QStringLiteral("qlight_overlay_bars"));
        area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        for (Qt::Orientation o : {Qt::Vertical, Qt::Horizontal}) {
            QScrollBar *native = o == Qt::Vertical ? area->verticalScrollBar() : area->horizontalScrollBar();
            auto *bar = new ScrollBar(o, area->viewport());
            bar->setFocusPolicy(Qt::NoFocus);
            m_bars[o == Qt::Vertical ? 0 : 1] = bar;
            QObject::connect(native, &QAbstractSlider::rangeChanged, this, [this] { syncAll(); });
            QObject::connect(native, &QAbstractSlider::valueChanged, this, [this, native, bar](int v) {
                if (m_syncing) return;
                m_syncing = true;
                bar->setValue(v);
                m_syncing = false;
                (void)native;
            });
            QObject::connect(bar, &QAbstractSlider::valueChanged, this, [this, native](int v) {
                if (m_syncing) return;
                m_syncing = true;
                native->setValue(v);
                m_syncing = false;
            });
        }
        area->viewport()->installEventFilter(this);
        syncAll();
    }

protected:
    bool eventFilter(QObject *obj, QEvent *e) override
    {
        if (obj == m_area->viewport() && (e->type() == QEvent::Resize || e->type() == QEvent::Show))
            syncAll();
        return false;
    }

private:
    void syncAll()
    {
        QWidget *vp = m_area->viewport();
        bool visible[2] = {};
        QScrollBar *natives[2] = {m_area->verticalScrollBar(), m_area->horizontalScrollBar()};
        m_syncing = true;
        for (int i = 0; i < 2; ++i) {
            m_bars[i]->setRange(natives[i]->minimum(), natives[i]->maximum());
            m_bars[i]->setPageStep(natives[i]->pageStep());
            m_bars[i]->setSingleStep(natives[i]->singleStep());
            m_bars[i]->setValue(natives[i]->value());
            visible[i] = natives[i]->maximum() > natives[i]->minimum();
        }
        m_syncing = false;
        const int t = kExtent;
        m_bars[0]->setGeometry(vp->width() - t, 0, t, vp->height() - (visible[1] ? t : 0));
        m_bars[1]->setGeometry(0, vp->height() - t, vp->width() - (visible[0] ? t : 0), t);
        for (int i = 0; i < 2; ++i) {
            m_bars[i]->setVisible(visible[i]);
            m_bars[i]->raise();
        }
    }

    QAbstractScrollArea *m_area;
    ScrollBar *m_bars[2] = {};
    bool m_syncing = false;
};

} // namespace

void ScrollBar::overlayIn(QAbstractScrollArea *area)
{
    if (!area->findChild<QObject *>(QStringLiteral("qlight_overlay_bars"), Qt::FindDirectChildrenOnly))
        new OverlayBars(area);
}

void ScrollBar::init()
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    setAutoFillBackground(false);
    // QScrollBar declares itself opaque, so Qt skips painting the parent underneath it; we only
    // paint the handle, and overlay bars must let the content below show through.
    setAttribute(Qt::WA_OpaquePaintEvent, false);

    m_thickAnim.setDuration(kAnimMs);
    m_thickAnim.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_thickAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_thickness = v.toReal();
        update();
    });
    connect(m_color, &detail::ColorAnimator::updated, this, qOverload<>(&QWidget::update));
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            [this] { syncTarget(false); });
    // range/value changes repaint through QAbstractSlider::update(); nothing else needed
    syncTarget(false);
}

QSize ScrollBar::sizeHint() const
{
    return orientation() == Qt::Vertical ? QSize(kExtent, 100) : QSize(100, kExtent);
}

QSize ScrollBar::minimumSizeHint() const
{
    return orientation() == Qt::Vertical ? QSize(kExtent, 2 * kMinHandle)
                                         : QSize(2 * kMinHandle, kExtent);
}

qreal ScrollBar::mainPos(const QPointF &p) const
{
    return orientation() == Qt::Vertical ? p.y() : p.x();
}

QRectF ScrollBar::handleRect() const
{
    const int range = maximum() - minimum();
    if (range <= 0)
        return {};

    const bool vertical = orientation() == Qt::Vertical;
    const qreal total = vertical ? height() : width();
    const qreal groove = total - 2 * kEndMargin;
    if (groove <= 0)
        return {};

    const qreal len = qBound(kMinHandle, groove * pageStep() / (range + pageStep()), groove);
    qreal t = qreal(sliderPosition() - minimum()) / range;
    if (invertedAppearance())
        t = 1 - t;
    const qreal start = kEndMargin + (groove - len) * t;
    const qreal cross = (kExtent - m_thickness) / 2.0;

    return vertical ? QRectF(cross, start, m_thickness, len) : QRectF(start, cross, len, m_thickness);
}

void ScrollBar::syncTarget(bool animate)
{
    const Palette &p = ThemeManager::instance().palette();
    const bool active = isEnabled() && (underMouse() || isSliderDown());

    m_color->setTarget(!isEnabled() ? p.textDisabled : active ? p.textSecondary : p.controlStrokeStrong,
                       animate);

    const qreal target = active ? kActiveThick : kRestThick;
    m_thickAnim.stop();
    if (!animate) {
        m_thickness = target;
        update();
        return;
    }
    m_thickAnim.setStartValue(m_thickness);
    m_thickAnim.setEndValue(target);
    m_thickAnim.start();
}

void ScrollBar::paintEvent(QPaintEvent *)
{
    const QRectF r = handleRect();
    if (r.isEmpty())
        return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(m_color->color());
    const qreal rad = qMin(r.width(), r.height()) / 2.0;
    p.drawRoundedRect(r, rad, rad);
}

void ScrollBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || maximum() <= minimum()) {
        e->ignore();
        return;
    }
    const QRectF h = handleRect();
    const qreal pos = mainPos(e->position());
    const qreal start = mainPos(h.topLeft());
    const qreal len = orientation() == Qt::Vertical ? h.height() : h.width();
    if (pos >= start && pos <= start + len) { // anywhere across the bar's thickness
        m_dragOffset = pos - start;
        setSliderDown(true);
    } else {
        triggerAction(pos < start ? SliderPageStepSub : SliderPageStepAdd);
    }
    syncTarget(true);
    e->accept();
}

void ScrollBar::mouseMoveEvent(QMouseEvent *e)
{
    if (!isSliderDown()) {
        e->ignore();
        return;
    }
    const bool vertical = orientation() == Qt::Vertical;
    const QRectF h = handleRect();
    const qreal len = vertical ? h.height() : h.width();
    const qreal total = vertical ? height() : width();
    const qreal travel = total - 2 * kEndMargin - len;
    if (travel <= 0)
        return;

    qreal t = (mainPos(e->position()) - m_dragOffset - kEndMargin) / travel;
    t = qBound(0.0, t, 1.0);
    if (invertedAppearance())
        t = 1 - t;
    setSliderPosition(minimum() + qRound(t * (maximum() - minimum())));
    e->accept();
}

void ScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && isSliderDown()) {
        setSliderDown(false);
        syncTarget(true);
        e->accept();
    }
}

void ScrollBar::enterEvent(QEnterEvent *e)
{
    QScrollBar::enterEvent(e);
    syncTarget(true);
}

void ScrollBar::leaveEvent(QEvent *e)
{
    QScrollBar::leaveEvent(e);
    syncTarget(true);
}

} // namespace QLight
