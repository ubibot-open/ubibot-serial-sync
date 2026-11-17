#pragma once

#include <QWidget>

#include "QLight/Export.h"

namespace QLight {

// Light-dismiss popup card anchored to a target widget: hosts any content widget, appears
// with a short slide + fade, closes when the user clicks elsewhere or presses Esc.
//
//   Flyout::open(button, new MyForm);                                   // owns and deletes it
//   Flyout::openMessage(button, "Remove item?", "This cannot be undone.");
//
// Placement::Auto tries below, above, right, left of the target; a fixed placement flips to
// the opposite side when it does not fit on screen.
class QLIGHT_EXPORT Flyout : public QWidget {
    Q_OBJECT
public:
    enum class Placement { Auto = -1, Bottom = 0, Top = 1, Right = 2, Left = 3 };
    Q_ENUM(Placement)

    // `content` becomes a child of the flyout and is deleted with it.
    explicit Flyout(QWidget *content, QWidget *parent = nullptr);

    QWidget *content() const { return m_content; }

    // Padding between the card edge and the content (default 16).
    void setPadding(int px);

    // Shows the flyout next to `target`. Repeated calls re-position it.
    void popup(QWidget *target, Placement placement = Placement::Auto);

    // Creates a flyout that deletes itself when it closes.
    static Flyout *open(QWidget *target, QWidget *content, Placement placement = Placement::Auto);
    static Flyout *openMessage(QWidget *target, const QString &title, const QString &text,
                               Placement placement = Placement::Auto);

signals:
    void closed();

protected:
    void paintEvent(QPaintEvent *) override;
    void hideEvent(QHideEvent *e) override;

private:
    void updateMargins();

    QWidget *m_content;
    int m_padding = 16;
};

} // namespace QLight
