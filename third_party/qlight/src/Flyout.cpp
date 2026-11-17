#include "QLight/Flyout.h"

#include "QLight/Shadow.h"
#include "QLight/Theme.h"

#include <QLabel>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScreen>
#include <QVBoxLayout>

#include "Placement.h"
#include "Util.h"

namespace QLight {

namespace {

constexpr int kGap = 6;
constexpr int kSlide = 8;
constexpr int kAnimMs = 150;
constexpr int kMaxTextWidth = 300;

ShadowSpec flyoutShadow()
{
    const Palette &p = ThemeManager::instance().palette();
    ShadowSpec s;
    s.radius = p.radiusCard;
    s.blur = 12;
    s.offsetY = 4;
    s.color = p.shadowColor;
    s.color.setAlpha(qMin(255, s.color.alpha() * 3 / 2));
    return s;
}

int shadowMargin() { return int(Shadow::extent(flyoutShadow())) + 2; }

} // namespace

Flyout::Flyout(QWidget *content, QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint), m_content(content)
{
    setAttribute(Qt::WA_TranslucentBackground);

    auto *lay = new QVBoxLayout(this);
    lay->setSizeConstraint(QLayout::SetFixedSize);
    content->setParent(this);
    lay->addWidget(content);
    updateMargins();

    // arbitrary content inherits the theme's text colour through the palette
    auto applyTheme = [this] {
        QPalette pal = palette();
        const QColor text = ThemeManager::instance().palette().textPrimary;
        for (auto role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText})
            pal.setColor(role, text);
        setPalette(pal);
        update();
    };
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, applyTheme);
    applyTheme();
}

void Flyout::setPadding(int px)
{
    m_padding = px;
    updateMargins();
}

void Flyout::updateMargins()
{
    const int m = shadowMargin() + m_padding;
    layout()->setContentsMargins(m, m, m, m);
}

void Flyout::popup(QWidget *target, Placement placement)
{
    layout()->activate();
    const int m = shadowMargin();
    const QSize card = size() - QSize(2 * m, 2 * m);

    const QRect targetRect(target->mapToGlobal(QPoint(0, 0)), target->size());
    QScreen *scr = target->screen();
    const QRect avail = scr ? scr->availableGeometry() : QRect(0, 0, 1 << 20, 1 << 20);
    const detail::Placed placed = detail::placeAround(targetRect, card, int(placement), kGap, avail);

    // slide in from the side the flyout grows out of
    QPoint offset;
    switch (placed.side) {
    case detail::Side::Bottom: offset = QPoint(0, -kSlide); break;
    case detail::Side::Top: offset = QPoint(0, kSlide); break;
    case detail::Side::Right: offset = QPoint(-kSlide, 0); break;
    case detail::Side::Left: offset = QPoint(kSlide, 0); break;
    }
    const QPoint finalPos = placed.topLeft - QPoint(m, m);

    if (!isVisible()) {
        move(finalPos + offset);
        setWindowOpacity(0.0);
        QWidget::show();
        auto *group = new QParallelAnimationGroup(this);
        auto *slide = new QPropertyAnimation(this, "pos");
        slide->setDuration(kAnimMs);
        slide->setEasingCurve(QEasingCurve::OutCubic);
        slide->setStartValue(finalPos + offset);
        slide->setEndValue(finalPos);
        group->addAnimation(slide);
        auto *fade = new QPropertyAnimation(this, "windowOpacity");
        fade->setDuration(kAnimMs);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);
        group->addAnimation(fade);
        group->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        move(finalPos);
    }
}

Flyout *Flyout::open(QWidget *target, QWidget *content, Placement placement)
{
    auto *f = new Flyout(content, target->window());
    f->setAttribute(Qt::WA_DeleteOnClose);
    f->popup(target, placement);
    return f;
}

Flyout *Flyout::openMessage(QWidget *target, const QString &title, const QString &text, Placement placement)
{
    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);
    const Palette &pal = ThemeManager::instance().palette();

    auto styled = [&](QLabel *l, const QColor &c) {
        l->setWordWrap(true);
        l->setMaximumWidth(kMaxTextWidth);
        QPalette p = l->palette();
        p.setColor(QPalette::WindowText, c);
        l->setPalette(p);
    };
    if (!title.isEmpty()) {
        auto *t = new QLabel(title);
        QFont f = t->font();
        f.setWeight(QFont::DemiBold);
        f.setPixelSize(16);
        t->setFont(f);
        styled(t, pal.textPrimary);
        lay->addWidget(t);
    }
    if (!text.isEmpty()) {
        auto *l = new QLabel(text);
        styled(l, pal.textPrimary);
        lay->addWidget(l);
    }
    return open(target, body, placement);
}

void Flyout::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int m = shadowMargin();
    const QRectF body = QRectF(rect()).adjusted(m, m, -m, -m);
    const qreal dpr = devicePixelRatioF();
    const qreal bw = detail::hairline(dpr);

    Shadow::draw(&p, body, flyoutShadow(), dpr);
    p.setBrush(pal.cardBg);
    p.setPen(QPen(pal.cardStroke, bw));
    p.drawRoundedRect(body.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), pal.radiusCard, pal.radiusCard);
}

void Flyout::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    emit closed();
}

} // namespace QLight
