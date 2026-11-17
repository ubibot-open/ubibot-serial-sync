#pragma once

#include <QScrollBar>
#include <QVariantAnimation>

#include "QLight/Export.h"

class QAbstractScrollArea;

namespace QLight {

namespace detail { class ColorAnimator; }

// Thin Fluent-style scroll bar: a 2px handle that widens to 6px on hover or
// drag. Hit-testing and painting are done here (not through QStyle), so it
// looks the same under any native style.
class QLIGHT_EXPORT ScrollBar : public QScrollBar {
    Q_OBJECT
public:
    explicit ScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);
    explicit ScrollBar(QWidget *parent = nullptr); // vertical

    // Replaces both scroll bars of `area` with QLight::ScrollBar instances.
    static void replaceIn(QAbstractScrollArea *area);

    // Hides the area's own scroll bars and floats thin QLight::ScrollBar handles over the
    // viewport instead ("overlay" scroll bars): they take no space, so the viewport can be
    // painted edge to edge. The hidden native bars keep driving the scrolling.
    static void overlayIn(QAbstractScrollArea *area);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    void init();
    QRectF handleRect() const; // in widget coordinates; empty when nothing to scroll
    void syncTarget(bool animate);
    qreal mainPos(const QPointF &p) const;

    detail::ColorAnimator *m_color;
    QVariantAnimation m_thickAnim;
    qreal m_thickness = 2;
    qreal m_dragOffset = 0;
};

} // namespace QLight
