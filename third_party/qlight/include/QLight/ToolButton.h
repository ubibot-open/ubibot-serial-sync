#pragma once

#include <QToolBar>
#include <QToolButton>

#include "QLight/Export.h"
#include "QLight/FluentIcon.h"

namespace QLight {

// Flat Fluent tool button: hover / pressed fill, accent fill while checked, icon and/or text,
// optional drop-down chevron (inline, or split for MenuButtonPopup). Inherits QToolButton, so
// actions, menus, popup modes, auto-raise, checkable / exclusive and all signals are unchanged.
//
// Painting is done by a QStyle (not a paintEvent), which is also how the buttons that QToolBar
// creates itself get the same look - see ToolBar.
class QLIGHT_EXPORT ToolButton : public QToolButton {
    Q_OBJECT
public:
    explicit ToolButton(QWidget *parent = nullptr);
    explicit ToolButton(const FluentIcon &icon, QWidget *parent = nullptr);

    // Convenience for setIcon(icon.icon()): a themed icon (selected = on-accent colour when checked).
    void setFluentIcon(const FluentIcon &icon);

protected:
    bool event(QEvent *e) override;

private:
    void init();
    void updateAutoAccessibleName();

    bool m_autoAccessibleName = false;
};

// QToolBar with Fluent tool buttons (the ones the bar creates for its actions and its overflow
// button), hairline separators and no native frame or handle. Not movable / floatable by default.
class QLIGHT_EXPORT ToolBar : public QToolBar {
    Q_OBJECT
public:
    explicit ToolBar(QWidget *parent = nullptr);
    explicit ToolBar(const QString &title, QWidget *parent = nullptr);

protected:
    bool event(QEvent *e) override;

private:
    void init();
    void styleChildren();
};

} // namespace QLight
