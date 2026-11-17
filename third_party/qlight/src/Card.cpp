#include "QLight/Card.h"

#include "QLight/Shadow.h"
#include "QLight/Theme.h"
#include "Util.h"

#include <QPainter>

#include <cmath>

namespace QLight {

namespace {

ShadowSpec cardShadow()
{
    const Palette &p = ThemeManager::instance().palette();
    ShadowSpec s;
    s.radius = p.radiusCard;
    s.blur = 8;
    s.offsetY = 2;
    s.color = p.shadowColor;
    return s;
}

} // namespace

Card::Card(QWidget *parent) : QFrame(parent)
{
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            qOverload<>(&QWidget::update));
    updateMargins();
}

int Card::shadowMargin() const
{
    return m_elevated ? int(QLight::Shadow::extent(cardShadow())) + 2 : 0;
}

void Card::setElevated(bool on)
{
    if (m_elevated == on)
        return;
    m_elevated = on;
    updateMargins();
    update();
}

void Card::setPadding(int px)
{
    m_padding = px;
    updateMargins();
}

void Card::updateMargins()
{
    const int m = shadowMargin() + m_padding;
    setContentsMargins(m, m, m, m);
}

void Card::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int sm = shadowMargin();
    const QRectF body = QRectF(rect()).adjusted(sm, sm, -sm, -sm);

    if (m_elevated)
        QLight::Shadow::draw(&p, body, cardShadow(), devicePixelRatioF());

    const qreal dpr = devicePixelRatioF();
    const qreal bw = detail::hairline(dpr);
    p.setBrush(pal.cardBg);
    p.setPen(QPen(pal.cardStroke, bw));
    p.drawRoundedRect(body.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), pal.radiusCard,
                      pal.radiusCard);
}

} // namespace QLight
