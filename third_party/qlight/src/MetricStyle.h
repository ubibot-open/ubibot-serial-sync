#pragma once

// Internal: a QProxyStyle that overrides a handful of pixel metrics / style hints so
// that Qt's own layout code (QMenu, QTabBar) uses our dimensions. All painting is
// done by the widgets themselves. Not part of the public API.

#include <QHash>
#include <QProxyStyle>

namespace QLight::detail {

class MetricStyle : public QProxyStyle {
public:
    // "fusion" as the base keeps behaviour identical on every platform.
    MetricStyle() : QProxyStyle(QStringLiteral("fusion")) {}

    void setMetric(PixelMetric m, int value) { m_metrics.insert(m, value); }
    void setHint(StyleHint h, int value) { m_hints.insert(h, value); }

    int pixelMetric(PixelMetric m, const QStyleOption *opt = nullptr, const QWidget *w = nullptr) const override
    {
        auto it = m_metrics.constFind(m);
        return it != m_metrics.constEnd() ? it.value() : QProxyStyle::pixelMetric(m, opt, w);
    }

    int styleHint(StyleHint h, const QStyleOption *opt = nullptr, const QWidget *w = nullptr,
                  QStyleHintReturn *ret = nullptr) const override
    {
        auto it = m_hints.constFind(h);
        return it != m_hints.constEnd() ? it.value() : QProxyStyle::styleHint(h, opt, w, ret);
    }

private:
    QHash<PixelMetric, int> m_metrics;
    QHash<StyleHint, int> m_hints;
};

} // namespace QLight::detail
