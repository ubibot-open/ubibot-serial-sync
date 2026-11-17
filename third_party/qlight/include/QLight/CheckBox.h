#pragma once

#include <QCheckBox>

#include "QLight/Export.h"

namespace QLight {

namespace detail { class ColorAnimator; }

// Fluent-style check box (supports tri-state). Inherits QCheckBox.
class QLIGHT_EXPORT CheckBox : public QCheckBox {
    Q_OBJECT
public:
    explicit CheckBox(QWidget *parent = nullptr);
    explicit CheckBox(const QString &text, QWidget *parent = nullptr);

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
    detail::ColorAnimator *m_mark;
    bool m_focusVisible = false;
};

} // namespace QLight
