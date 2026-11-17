#pragma once

#include <QColor>
#include <QPushButton>
#include <QVariantAnimation>

#include "QLight/Export.h"
#include "QLight/FluentIcon.h"

namespace QLight {

class QLIGHT_EXPORT PushButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(Variant variant READ variant WRITE setVariant)
public:
    enum Variant { Standard, Accent, Subtle };
    Q_ENUM(Variant)

    explicit PushButton(QWidget *parent = nullptr);
    explicit PushButton(const QString &text, QWidget *parent = nullptr);
    PushButton(const QString &text, Variant variant, QWidget *parent = nullptr);

    Variant variant() const { return m_variant; }
    void setVariant(Variant v);

    // Vector icon drawn in the button's current text colour (so it follows hover, pressed,
    // disabled and accent states). Takes precedence over QPushButton::setIcon().
    FluentIcon fluentIcon() const { return m_fluentIcon; }
    void setFluentIcon(const FluentIcon &icon);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }

protected:
    bool event(QEvent *e) override;
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void changeEvent(QEvent *e) override;

private:
    void updateAutoAccessibleName();
    void init();
    bool accentLook() const;
    QColor targetFill() const;
    QColor targetText() const;
    void syncTarget(bool animate);

    Variant m_variant = Standard;
    bool m_autoAccessibleName = false;
    FluentIcon m_fluentIcon;
    bool m_focusVisible = false;
    QColor m_fill, m_text;
    QColor m_fromFill, m_fromText, m_toFill, m_toText;
    QVariantAnimation m_anim;
};

} // namespace QLight
