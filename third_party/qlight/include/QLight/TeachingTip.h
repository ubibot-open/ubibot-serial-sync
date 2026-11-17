#pragma once

#include <QWidget>

#include "QLight/Export.h"
#include "QLight/Flyout.h"
#include "QLight/FluentIcon.h"

class QLabel;

namespace QLight {

class IconWidget;
class PushButton;

// Callout with a tail pointing at a target widget: optional icon, title, text, an action
// button and a close button. Use it to introduce a feature or explain a control.
//
//   auto *tip = new QLight::TeachingTip("Try this", "Drag the handle to reorder.", window);
//   tip->setIcon(QLight::Icon::Info);
//   tip->setActionText("Got it");
//   tip->popup(handleButton);
//
// By default a click elsewhere dismisses it (light dismiss). setLightDismiss(false) keeps it
// until the user closes it (or setAutoCloseMs() elapses).
class QLIGHT_EXPORT TeachingTip : public QWidget {
    Q_OBJECT
public:
    explicit TeachingTip(const QString &title, const QString &text = {}, QWidget *parent = nullptr);

    QString title() const;
    void setTitle(const QString &title);
    void setText(const QString &text);
    void setIcon(const FluentIcon &icon);

    void setActionText(const QString &text); // empty removes the button
    void setClosable(bool on);               // close button, default true
    bool isClosable() const { return m_closable; }
    void setLightDismiss(bool on);           // default true
    bool isLightDismiss() const { return m_lightDismiss; }
    void setAutoCloseMs(int ms);             // 0 = never (default)

    // Shows the tip next to `target` with its tail pointing at it. Delete-on-close is off:
    // the caller owns the tip unless created through create().
    void popup(QWidget *target, Flyout::Placement placement = Flyout::Placement::Auto);

    static TeachingTip *create(QWidget *target, const QString &title, const QString &text = {},
                               Flyout::Placement placement = Flyout::Placement::Auto);

signals:
    void actionTriggered();
    void closed();

protected:
    void paintEvent(QPaintEvent *) override;
    void hideEvent(QHideEvent *e) override;

private:
    void applyFlags();
    void applyMargins(int tailSide); // 0..3 = detail::Side, -1 = none
    void applyTheme();

    IconWidget *m_iconWidget;
    QLabel *m_title;
    QLabel *m_text;
    PushButton *m_action;
    PushButton *m_close;
    bool m_closable = true;
    bool m_lightDismiss = true;
    int m_autoCloseMs = 0;
    int m_side = -1;      // side of the tip the tail is on: which edge faces the target
    qreal m_tailPos = 0;  // tail centre along that edge, in widget coordinates
};

} // namespace QLight
