#pragma once

#include <QComboBox>

#include "QLight/Export.h"

namespace QLight {

namespace detail {
class ColorAnimator;
class ComboPopup;
} // namespace detail

// Fluent-style drop-down list. Inherits QComboBox, so the model/items API and
// the activated/currentIndexChanged signals work as usual.
//
// The popup is a rounded, shadowed flyout drawn by this library. Editable
// combo boxes are not restyled yet and fall back to QComboBox behaviour.
class QLIGHT_EXPORT ComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit ComboBox(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void showPopup() override;
    void hidePopup() override;

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void changeEvent(QEvent *e) override;

private:
    friend class detail::ComboPopup;

    void syncTarget(bool animate);
    bool popupOpen() const;
    void popupChosen(int row);

    detail::ColorAnimator *m_fill;
    detail::ComboPopup *m_popup = nullptr;
    bool m_focusVisible = false;
};

} // namespace QLight
