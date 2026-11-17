#include "QLight/ToolButton.h"

#include "QLight/Theme.h"

#include <QChildEvent>
#include <QEvent>

#include "ToolStyle.h"

namespace QLight {

namespace {

// One shared style object per application would be enough, but QStyle objects need an owner and
// widgets may be destroyed in any order; give every widget its own cheap instance instead.
detail::ToolButtonStyle *newStyle(QObject *owner)
{
    auto *st = new detail::ToolButtonStyle;
    st->setParent(owner);
    return st;
}

} // namespace

ToolButton::ToolButton(QWidget *parent) : QToolButton(parent) { init(); }

ToolButton::ToolButton(const FluentIcon &icon, QWidget *parent) : QToolButton(parent)
{
    init();
    setFluentIcon(icon);
}

void ToolButton::init()
{
    setStyle(newStyle(this));
    setAutoRaise(true);
    setIconSize(QSize(16, 16));
    setAttribute(Qt::WA_Hover);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, qOverload<>(&QWidget::update));
}

void ToolButton::setFluentIcon(const FluentIcon &icon)
{
    setIcon(icon.icon());
    updateAutoAccessibleName();
}

// An icon-only button has no text for a screen reader: use the tooltip as its accessible name.
void ToolButton::updateAutoAccessibleName()
{
    if (!accessibleName().isEmpty() && !m_autoAccessibleName)
        return;
    if (text().isEmpty() && !toolTip().isEmpty()) {
        setAccessibleName(toolTip());
        m_autoAccessibleName = true;
    } else if (m_autoAccessibleName) {
        setAccessibleName(QString());
        m_autoAccessibleName = false;
    }
}

bool ToolButton::event(QEvent *e)
{
    const bool handled = QToolButton::event(e);
    if (e->type() == QEvent::ToolTipChange)
        updateAutoAccessibleName();
    return handled;
}

// ---------------------------------------------------------------------------

ToolBar::ToolBar(QWidget *parent) : QToolBar(parent) { init(); }
ToolBar::ToolBar(const QString &title, QWidget *parent) : QToolBar(title, parent) { init(); }

void ToolBar::init()
{
    setStyle(newStyle(this));
    setMovable(false);
    setFloatable(false);
    setIconSize(QSize(16, 16));
    setAutoFillBackground(false);
    styleChildren();
}

// The buttons a QToolBar creates for its actions (and its overflow "extension" button) are plain
// QToolButtons created later with the application style; give them ours as they appear.
void ToolBar::styleChildren()
{
    for (auto *b : findChildren<QToolButton *>(QString(), Qt::FindDirectChildrenOnly)) {
        if (!dynamic_cast<detail::ToolButtonStyle *>(b->style())) {
            b->setStyle(newStyle(b));
            b->setAttribute(Qt::WA_Hover);
        }
    }
    // the extension button lives inside the toolbar's layout widget
    for (auto *b : findChildren<QToolButton *>()) {
        if (!dynamic_cast<detail::ToolButtonStyle *>(b->style())) {
            b->setStyle(newStyle(b));
            b->setAttribute(Qt::WA_Hover);
        }
    }
}

bool ToolBar::event(QEvent *e)
{
    const bool handled = QToolBar::event(e);
    switch (e->type()) {
    case QEvent::ChildPolished:
    case QEvent::ChildAdded:
    case QEvent::ActionAdded:
    case QEvent::Show:
    case QEvent::LayoutRequest:
        styleChildren();
        break;
    default:
        break;
    }
    return handled;
}

} // namespace QLight
