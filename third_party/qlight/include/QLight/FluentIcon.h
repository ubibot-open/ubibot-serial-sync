#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QWidget>

#include <memory>

#include "QLight/Export.h"

namespace QLight {

// Built-in icons: a small subset of Microsoft's Fluent UI System Icons
// (MIT, 20px "regular" style), rendered from SVG and tinted at paint time.
enum class Icon {
    Add, Subtract, Dismiss, Checkmark,
    ChevronDown, ChevronUp, ChevronLeft, ChevronRight,
    ArrowLeft, ArrowRight, ArrowUp, ArrowDown,
    Search, Settings, Home, Navigation, MoreHorizontal,
    Info, Warning, ErrorCircle, CheckmarkCircle, QuestionCircle,
    Delete, Edit, Copy, Save, Open, Share,
    Folder, Document, Person, Mail, Calendar, Clock,
    Heart, Star, Eye, EyeOff, LockClosed, Globe,
    Play, Pause, Stop, Filter, ArrowSync,
    WeatherMoon, WeatherSunny,
    Count_ // number of icons; not an icon
};

// A monochrome vector icon. Cheap to copy (shared, immutable data).
//
//   button->setFluentIcon(FluentIcon(Icon::Add));     // PushButton follows its text colour
//   QIcon qi = FluentIcon(Icon::Search).icon();       // themed QIcon for any Qt API
//   QPixmap px = FluentIcon(Icon::Info).pixmap(16, Qt::red);
//
// Custom icons: FluentIcon::fromFile("path/or/:/resource.svg"). The SVG is used
// as an alpha mask, so any fill colour in the file is ignored.
class QLIGHT_EXPORT FluentIcon {
public:
    FluentIcon() = default;
    FluentIcon(Icon icon); // NOLINT: implicit on purpose, e.g. setFluentIcon(Icon::Add)

    static FluentIcon fromFile(const QString &svgPath);
    static const char *name(Icon icon); // e.g. "chevron_down"

    bool isNull() const { return !m_data; }

    // Rendered at `size` logical pixels for the given device pixel ratio, tinted with `color`.
    QPixmap pixmap(const QSize &size, const QColor &color, qreal dpr = 1.0) const;
    QPixmap pixmap(int size, const QColor &color, qreal dpr = 1.0) const
    {
        return pixmap(QSize(size, size), color, dpr);
    }

    // QIcon whose colour follows the current theme (text colour; disabled and
    // selected modes use the matching theme colours), re-evaluated on every paint.
    QIcon icon() const;
    // QIcon with a fixed colour.
    QIcon icon(const QColor &color) const;

    struct Data; // implementation detail

private:
    std::shared_ptr<const Data> m_data;
};

// Small widget that shows a FluentIcon in the theme's text colour.
class QLIGHT_EXPORT IconWidget : public QWidget {
    Q_OBJECT
public:
    explicit IconWidget(QWidget *parent = nullptr);
    explicit IconWidget(const FluentIcon &icon, int size = 20, QWidget *parent = nullptr);

    FluentIcon fluentIcon() const { return m_icon; }
    void setFluentIcon(const FluentIcon &icon);
    int iconSize() const { return m_size; }
    void setIconSize(int size);

    // Overrides the theme colour; pass an invalid QColor to go back to it.
    void setColor(const QColor &color);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    FluentIcon m_icon;
    int m_size = 20;
    QColor m_color;
};

} // namespace QLight
