#pragma once

#include <QLineEdit>

#include "QLight/Export.h"

namespace QLight {

namespace detail { class ColorAnimator; }

// Single-line text input: hover/focus fill transition and an accent underline
// while focused. Inherits QLineEdit, so the whole QLineEdit API still applies.
class QLIGHT_EXPORT LineEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit LineEdit(QWidget *parent = nullptr);
    explicit LineEdit(const QString &text, QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void changeEvent(QEvent *e) override;

private:
    void init();
    void applyPalette();
    void syncTarget(bool animate);

    detail::ColorAnimator *m_fill;
};

} // namespace QLight
