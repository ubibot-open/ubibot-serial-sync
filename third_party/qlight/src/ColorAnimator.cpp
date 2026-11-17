#include "ColorAnimator.h"

#include "Util.h"

namespace QLight::detail {

ColorAnimator::ColorAnimator(QObject *parent, int durationMs) : QObject(parent)
{
    m_anim.setDuration(durationMs);
    m_anim.setEasingCurve(QEasingCurve::OutCubic);
    m_anim.setStartValue(0.0);
    m_anim.setEndValue(1.0);
    connect(&m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_current = lerpColor(m_from, m_to, v.toReal());
        emit updated();
    });
}

void ColorAnimator::setTarget(const QColor &target, bool animate)
{
    if (target == m_to && m_current.isValid())
        return;
    m_anim.stop();
    m_to = target;
    if (!animate || !m_current.isValid()) {
        m_current = target;
        emit updated();
        return;
    }
    m_from = m_current;
    m_anim.start();
}

} // namespace QLight::detail
