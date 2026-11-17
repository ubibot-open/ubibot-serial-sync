#pragma once

#include <QRadioButton>
#include <QVariantAnimation>

#include "QLight/Export.h"

namespace QLight {

namespace detail { class ColorAnimator; }

// Fluent-style radio button. Inherits QRadioButton, so auto-exclusive groups,
// QButtonGroup and arrow-key navigation between siblings work as usual.
class QLIGHT_EXPORT RadioButton : public QRadioButton {
    Q_OBJECT
public:
    explicit RadioButton(QWidget *parent = nullptr);
    explicit RadioButton(const QString &text, QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }

protected:
    void paintEvent(QPaintEvent *) override;
    bool hitButton(const QPoint &pos) const override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void changeEvent(QEvent *e) override;

private:
    void init();
    void syncTarget(bool animate);

    detail::ColorAnimator *m_fill;
    detail::ColorAnimator *m_stroke;
    detail::ColorAnimator *m_dotColor;
    QVariantAnimation m_dotAnim;
    qreal m_dot = 0; // diameter of the inner dot
    bool m_focusVisible = false;
};

} // namespace QLight
