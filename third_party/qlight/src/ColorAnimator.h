#pragma once

// Internal: animates a QColor toward a target. Not part of the public API.

#include <QColor>
#include <QObject>
#include <QVariantAnimation>

namespace QLight::detail {

class ColorAnimator : public QObject {
    Q_OBJECT
public:
    explicit ColorAnimator(QObject *parent = nullptr, int durationMs = 120);

    QColor color() const { return m_current; }

    // animate=false snaps immediately (used for theme changes and initial state).
    void setTarget(const QColor &target, bool animate);

signals:
    void updated();

private:
    QVariantAnimation m_anim;
    QColor m_current, m_from, m_to;
};

} // namespace QLight::detail
