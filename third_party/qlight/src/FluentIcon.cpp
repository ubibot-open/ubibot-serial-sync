#include "QLight/FluentIcon.h"

#include "QLight/Theme.h"

#include <QCoreApplication>
#include <QFile>
#include <QIconEngine>
#include <QImage>
#include <QPainter>
#include <QPixmapCache>
#include <QSvgRenderer>

#include <cmath>

namespace QLight {

namespace {

// Keep in sync with QLight::Icon; the file name is resources/icons/<name>.svg.
constexpr const char *kNames[] = {
    "add", "subtract", "dismiss", "checkmark",
    "chevron_down", "chevron_up", "chevron_left", "chevron_right",
    "arrow_left", "arrow_right", "arrow_up", "arrow_down",
    "search", "settings", "home", "navigation", "more_horizontal",
    "info", "warning", "error_circle", "checkmark_circle", "question_circle",
    "delete", "edit", "copy", "save", "open", "share",
    "folder", "document", "person", "mail", "calendar", "clock",
    "heart", "star", "eye", "eye_off", "lock_closed", "globe",
    "play", "pause", "stop", "filter", "arrow_sync",
    "weather_moon", "weather_sunny",
};
static_assert(sizeof(kNames) / sizeof(kNames[0]) == size_t(Icon::Count_),
              "icon name table out of sync with QLight::Icon");

} // namespace

struct FluentIcon::Data {
    QString key;                // cache key: "b:<name>" or "f:<path>"
    std::unique_ptr<QSvgRenderer> renderer;
};

const char *FluentIcon::name(Icon icon)
{
    const auto i = size_t(icon);
    return i < size_t(Icon::Count_) ? kNames[i] : "";
}

FluentIcon::FluentIcon(Icon icon)
{
    // one shared Data (and QSvgRenderer) per built-in icon, released at application exit
    static std::shared_ptr<const Data> cache[size_t(Icon::Count_)];
    static const bool registered = [] {
        qAddPostRoutine([] {
            for (auto &c : cache)
                c.reset();
        });
        return true;
    }();
    Q_UNUSED(registered);
    const auto i = size_t(icon);
    if (i >= size_t(Icon::Count_))
        return;
    if (!cache[i]) {
        auto d = std::make_shared<Data>();
        d->key = QStringLiteral("b:") + QLatin1String(kNames[i]);
        d->renderer = std::make_unique<QSvgRenderer>(
            QStringLiteral(":/qlight/icons/") + QLatin1String(kNames[i]) + QStringLiteral(".svg"));
        if (!d->renderer->isValid())
            return;
        cache[i] = std::move(d);
    }
    m_data = cache[i];
}

FluentIcon FluentIcon::fromFile(const QString &svgPath)
{
    FluentIcon result;
    auto d = std::make_shared<Data>();
    d->key = QStringLiteral("f:") + svgPath;
    d->renderer = std::make_unique<QSvgRenderer>(svgPath);
    if (d->renderer->isValid())
        result.m_data = std::move(d);
    return result;
}

QPixmap FluentIcon::pixmap(const QSize &size, const QColor &color, qreal dpr) const
{
    if (!m_data || size.isEmpty() || !color.isValid())
        return {};

    const QString key = QStringLiteral("qlight/%1/%2x%3/%4/%5")
                            .arg(m_data->key)
                            .arg(size.width())
                            .arg(size.height())
                            .arg(color.rgba(), 0, 16)
                            .arg(int(std::round(dpr * 100)));
    QPixmap pm;
    if (QPixmapCache::find(key, &pm))
        return pm;

    const QSize px(int(std::ceil(size.width() * dpr)), int(std::ceil(size.height() * dpr)));
    QImage img(px, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    {
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        // keep the icon's aspect ratio, centred in the requested box
        QSizeF natural = m_data->renderer->defaultSize();
        if (natural.isEmpty())
            natural = QSizeF(1, 1);
        const qreal scale = qMin(px.width() / natural.width(), px.height() / natural.height());
        const QSizeF target(natural.width() * scale, natural.height() * scale);
        m_data->renderer->render(&p, QRectF(QPointF((px.width() - target.width()) / 2,
                                                    (px.height() - target.height()) / 2), target));
        // tint: the SVG only provides the alpha mask
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(img.rect(), color);
    }
    pm = QPixmap::fromImage(img);
    pm.setDevicePixelRatio(dpr);
    QPixmapCache::insert(key, pm);
    return pm;
}

namespace {

class ThemedIconEngine : public QIconEngine {
public:
    ThemedIconEngine(const FluentIcon &icon, const QColor &fixed) : m_icon(icon), m_fixed(fixed) {}

    QIconEngine *clone() const override { return new ThemedIconEngine(*this); }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State) override
    {
        const qreal dpr = painter->device()->devicePixelRatioF();
        const QPixmap pm = m_icon.pixmap(rect.size(), colorFor(mode), dpr);
        painter->drawPixmap(rect.topLeft(), pm);
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
        QPixmap pm(size);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        paint(&p, QRect(QPoint(0, 0), size), mode, state);
        return pm;
    }

    QSize actualSize(const QSize &size, QIcon::Mode, QIcon::State) override { return size; }

private:
    QColor colorFor(QIcon::Mode mode) const
    {
        if (m_fixed.isValid())
            return mode == QIcon::Disabled ? QColor(m_fixed.red(), m_fixed.green(), m_fixed.blue(), m_fixed.alpha() / 3)
                                           : m_fixed;
        const Palette &p = ThemeManager::instance().palette();
        switch (mode) {
        case QIcon::Disabled: return p.textDisabled;
        case QIcon::Selected: return p.textOnAccent;
        default: return p.textPrimary;
        }
    }

    FluentIcon m_icon;
    QColor m_fixed;
};

} // namespace

QIcon FluentIcon::icon() const
{
    return m_data ? QIcon(new ThemedIconEngine(*this, QColor())) : QIcon();
}

QIcon FluentIcon::icon(const QColor &color) const
{
    return m_data ? QIcon(new ThemedIconEngine(*this, color)) : QIcon();
}

// ---------------------------------------------------------------------------

IconWidget::IconWidget(QWidget *parent) : QWidget(parent)
{
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            qOverload<>(&QWidget::update));
}

IconWidget::IconWidget(const FluentIcon &icon, int size, QWidget *parent)
    : IconWidget(parent)
{
    m_icon = icon;
    m_size = size;
}

void IconWidget::setFluentIcon(const FluentIcon &icon)
{
    m_icon = icon;
    update();
}

void IconWidget::setIconSize(int size)
{
    m_size = size;
    updateGeometry();
    update();
}

void IconWidget::setColor(const QColor &color)
{
    m_color = color;
    update();
}

QSize IconWidget::sizeHint() const { return {m_size, m_size}; }

void IconWidget::paintEvent(QPaintEvent *)
{
    if (m_icon.isNull())
        return;
    QColor c = m_color.isValid() ? m_color : ThemeManager::instance().palette().textPrimary;
    if (!isEnabled())
        c = ThemeManager::instance().palette().textDisabled;
    QPainter p(this);
    const QPixmap pm = m_icon.pixmap(QSize(m_size, m_size), c, devicePixelRatioF());
    p.drawPixmap((width() - m_size) / 2, (height() - m_size) / 2, pm);
}

} // namespace QLight
