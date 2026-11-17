#include "Accessibility.h"

#include "QLight/Calendar.h"
#include "QLight/FluentIcon.h"
#include "QLight/InfoBar.h"
#include "QLight/TeachingTip.h"

namespace QLight::detail {

namespace {

QAccessibleInterface *factory(const QString &className, QObject *object)
{
    auto *w = qobject_cast<QWidget *>(object);
    if (!w)
        return nullptr;

    if (className == QLatin1String("QLight::InfoBar")) {
        // announced as an alert message; the name is "Title. Message"
        return new FixedRoleAccessible(w, QAccessible::AlertMessage, [](QWidget *w) {
            auto *bar = static_cast<InfoBar *>(w);
            return bar->message().isEmpty() ? bar->title() : bar->title() + QStringLiteral(". ") + bar->message();
        });
    }
    if (className == QLatin1String("QLight::TeachingTip")) {
        return new FixedRoleAccessible(w, QAccessible::Dialog,
                                       [](QWidget *w) { return static_cast<TeachingTip *>(w)->title(); });
    }
    if (className == QLatin1String("QLight::Flyout"))
        return new FixedRoleAccessible(w, QAccessible::Dialog);
    if (className == QLatin1String("QLight::CalendarWidget")) {
        // no per-day child interfaces yet: the calendar is one grouping named by its month and selected day
        return new FixedRoleAccessible(w, QAccessible::Grouping, [](QWidget *w) {
            auto *c = static_cast<CalendarWidget *>(w);
            return QStringLiteral("Calendar, %1 %2, selected %3")
                .arg(QLocale().monthName(c->currentPageMonth()))
                .arg(c->currentPageYear())
                .arg(QLocale().toString(c->selectedDate(), QLocale::LongFormat));
        });
    }
    if (className == QLatin1String("QLight::IconWidget")) // decorative unless given an accessible name
        return new FixedRoleAccessible(w, QAccessible::Graphic);
    return nullptr;
}

} // namespace

void installAccessibility()
{
    static bool installed = false;
    if (installed)
        return;
    installed = true;
    QAccessible::installFactory(&factory);
}

} // namespace QLight::detail
