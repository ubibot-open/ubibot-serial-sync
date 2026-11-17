#pragma once

#include <QMenu>

#include "QLight/Export.h"

namespace QLight {

// Fluent-style context / drop-down menu: rounded, shadowed, with a hover
// highlight, an icon/check column and right-aligned shortcuts. Inherits QMenu,
// so actions, submenus, exec()/popup(), signals and keyboard navigation work as usual.
//
// Sub-menus should be QLight::Menu too: create them with addSubMenu(), or pass
// your own Menu to addMenu(). (A plain QMenu added with addMenu() keeps the
// native look.)
class QLIGHT_EXPORT Menu : public QMenu {
    Q_OBJECT
public:
    explicit Menu(QWidget *parent = nullptr);
    explicit Menu(const QString &title, QWidget *parent = nullptr);

    // Adds a sub-menu (a QLight::Menu) with the given title and optional icon.
    Menu *addSubMenu(const QString &title, const QIcon &icon = {});

    // Space around the card reserved for the drop shadow, in logical pixels. The
    // menu widget is larger than the visible card by this much on every side;
    // exec()/popup() positions are corrected so the card (not the widget) lands there.
    static int shadowMargin();

protected:
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;

private:
    void init();
    bool hasIconColumn() const;
    bool isSubMenu() const;
    QMenu *parentMenu() const;

    bool m_shifted = false;
};

} // namespace QLight
