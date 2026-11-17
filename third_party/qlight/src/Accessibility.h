#pragma once

// Internal: accessibility (QAccessible) support shared by the widgets. Not part of the public API.

#include <QAccessible>
#include <QAccessibleWidget>
#include <QWidget>

namespace QLight::detail {

// A widget with a fixed accessible role and an optional computed name.
class FixedRoleAccessible : public QAccessibleWidget {
public:
    using NameFn = QString (*)(QWidget *);

    FixedRoleAccessible(QWidget *w, QAccessible::Role role, NameFn nameFn = nullptr)
        : QAccessibleWidget(w, role), m_nameFn(nameFn)
    {
    }

    QString text(QAccessible::Text t) const override
    {
        if (t == QAccessible::Name) {
            QString n = widget()->accessibleName();
            if (n.isEmpty() && m_nameFn)
                n = m_nameFn(widget());
            return n;
        }
        return QAccessibleWidget::text(t);
    }

private:
    NameFn m_nameFn;
};

// Registers the factory for the simple widgets (InfoBar, Flyout, TeachingTip, IconWidget).
// Idempotent; called when the ThemeManager is created, i.e. before any QLight widget exists.
void installAccessibility();

} // namespace QLight::detail
