#include "QLight/TeachingTip.h"

#include "QLight/PushButton.h"
#include "QLight/Shadow.h"
#include "QLight/Theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

#include "Placement.h"
#include "Util.h"

namespace QLight {

namespace {

constexpr int kCardWidth = 320;
constexpr int kPadding = 16;
constexpr int kTail = 10;        // tail height
constexpr int kTailHalf = 10;    // half of the tail's base
constexpr int kSlide = 8;
constexpr int kAnimMs = 150;

// which edge of the tip carries the tail
enum Edge { Top = 0, Bottom = 1, LeftEdge = 2, RightEdge = 3 };

ShadowSpec tipShadow()
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

int shadowMargin() { return int(Shadow::extent(tipShadow())) + 2; }

Edge edgeFacing(detail::Side popupSide)
{
    // popup below the target -> tail on its top edge, and so on
    switch (popupSide) {
    case detail::Side::Bottom: return Top;
    case detail::Side::Top: return Bottom;
    case detail::Side::Right: return LeftEdge;
    default: return RightEdge;
    }
}

} // namespace

TeachingTip::TeachingTip(const QString &title, const QString &text, QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
      m_iconWidget(new IconWidget(this)), m_title(new QLabel(title, this)), m_text(new QLabel(text, this)),
      m_action(new PushButton(this)), m_close(new PushButton(this))
{
    setAttribute(Qt::WA_TranslucentBackground);

    m_iconWidget->setIconSize(20);
    m_iconWidget->setFixedSize(20, 20);
    m_iconWidget->hide();

    QFont tf = m_title->font();
    tf.setWeight(QFont::DemiBold);
    tf.setPixelSize(16);
    m_title->setFont(tf);
    m_title->setWordWrap(true);
    m_text->setWordWrap(true);
    m_text->setVisible(!text.isEmpty());

    m_close->setVariant(PushButton::Subtle);
    m_close->setFluentIcon(Icon::Dismiss);
    m_close->setFixedSize(28, 28);
    m_close->setAccessibleName(tr("Close"));
    connect(m_close, &QPushButton::clicked, this, &QWidget::close);

    m_action->hide();
    connect(m_action, &QPushButton::clicked, this, [this] {
        emit actionTriggered();
        close();
    });

    auto *head = new QHBoxLayout;
    head->setSpacing(8);
    head->addWidget(m_iconWidget, 0, Qt::AlignTop);
    head->addWidget(m_title, 1);
    head->addWidget(m_close, 0, Qt::AlignTop);

    auto *lay = new QVBoxLayout(this);
    lay->setSpacing(8);
    lay->addLayout(head);
    lay->addWidget(m_text);
    lay->addWidget(m_action, 0, Qt::AlignLeft);
    applyMargins(-1);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] {
        applyTheme();
        update();
    });
    applyTheme();
}

void TeachingTip::applyTheme()
{
    const Palette &p = ThemeManager::instance().palette();
    QPalette tp = m_title->palette();
    tp.setColor(QPalette::WindowText, p.textPrimary);
    m_title->setPalette(tp);
    QPalette xp = m_text->palette();
    xp.setColor(QPalette::WindowText, p.textSecondary);
    m_text->setPalette(xp);
    m_iconWidget->setColor(p.accent);
}

QString TeachingTip::title() const { return m_title->text(); }

void TeachingTip::setTitle(const QString &title) { m_title->setText(title); }

void TeachingTip::setText(const QString &text)
{
    m_text->setText(text);
    m_text->setVisible(!text.isEmpty());
}

void TeachingTip::setIcon(const FluentIcon &icon)
{
    m_iconWidget->setFluentIcon(icon);
    m_iconWidget->setVisible(!icon.isNull());
}

void TeachingTip::setActionText(const QString &text)
{
    m_action->setText(text);
    m_action->setVisible(!text.isEmpty());
}

void TeachingTip::setClosable(bool on)
{
    m_closable = on;
    m_close->setVisible(on);
}

void TeachingTip::setLightDismiss(bool on)
{
    m_lightDismiss = on;
    applyFlags();
}

void TeachingTip::setAutoCloseMs(int ms) { m_autoCloseMs = ms; }

void TeachingTip::applyFlags()
{
    if (isVisible())
        return;
    Qt::WindowFlags f = Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint;
    f |= m_lightDismiss ? Qt::Popup : (Qt::Tool | Qt::WindowStaysOnTopHint);
    setWindowFlags(f);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating, !m_lightDismiss);
}

void TeachingTip::applyMargins(int side)
{
    m_side = side;
    const int m = shadowMargin() + kPadding;
    layout()->setContentsMargins(m + (side == LeftEdge ? kTail : 0), m + (side == Top ? kTail : 0),
                                 m + (side == RightEdge ? kTail : 0), m + (side == Bottom ? kTail : 0));
    // the content column is kCardWidth - 2 * kPadding wide; the rest is shadow and tail
    const int w = kCardWidth + 2 * shadowMargin() + ((side == LeftEdge || side == RightEdge) ? kTail : 0);
    setFixedWidth(w);
    layout()->activate();
    setFixedHeight(qMax(1, layout()->totalHeightForWidth(w)));
}

void TeachingTip::popup(QWidget *target, Flyout::Placement placement)
{
    applyFlags();

    // 1) size the card without a tail to choose a side
    applyMargins(-1);
    const int m = shadowMargin();
    const QSize card(kCardWidth, height() - 2 * m);

    const QRect targetRect(target->mapToGlobal(QPoint(0, 0)), target->size());
    QScreen *scr = target->screen();
    const QRect avail = scr ? scr->availableGeometry() : QRect(0, 0, 1 << 20, 1 << 20);
    const detail::Placed placed = detail::placeAround(targetRect, card, int(placement), kTail + 2, avail);

    // 2) add the tail on the edge that faces the target and position the widget
    const Edge edge = edgeFacing(placed.side);
    applyMargins(edge);
    const QPoint widgetTopLeft = placed.topLeft - QPoint(m + (edge == LeftEdge ? kTail : 0), m + (edge == Top ? kTail : 0));

    const QPoint targetCenter = targetRect.center();
    const int inset = kPadding + kTailHalf;
    if (edge == Top || edge == Bottom)
        m_tailPos = qBound<qreal>(m + inset, targetCenter.x() - widgetTopLeft.x(), width() - m - inset);
    else
        m_tailPos = qBound<qreal>(m + inset, targetCenter.y() - widgetTopLeft.y(), height() - m - inset);

    QPoint offset;
    switch (placed.side) {
    case detail::Side::Bottom: offset = QPoint(0, -kSlide); break;
    case detail::Side::Top: offset = QPoint(0, kSlide); break;
    case detail::Side::Right: offset = QPoint(-kSlide, 0); break;
    case detail::Side::Left: offset = QPoint(kSlide, 0); break;
    }
    move(widgetTopLeft + offset);
    setWindowOpacity(0.0);
    show();
    auto *group = new QParallelAnimationGroup(this);
    auto *slide = new QPropertyAnimation(this, "pos");
    slide->setDuration(kAnimMs);
    slide->setEasingCurve(QEasingCurve::OutCubic);
    slide->setStartValue(widgetTopLeft + offset);
    slide->setEndValue(widgetTopLeft);
    auto *fade = new QPropertyAnimation(this, "windowOpacity");
    fade->setDuration(kAnimMs);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    group->addAnimation(slide);
    group->addAnimation(fade);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    if (m_autoCloseMs > 0)
        QTimer::singleShot(m_autoCloseMs, this, &QWidget::close);
}

TeachingTip *TeachingTip::create(QWidget *target, const QString &title, const QString &text,
                                 Flyout::Placement placement)
{
    auto *tip = new TeachingTip(title, text, target->window());
    tip->setAttribute(Qt::WA_DeleteOnClose);
    tip->popup(target, placement);
    return tip;
}

void TeachingTip::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal dpr = devicePixelRatioF();
    const qreal bw = detail::hairline(dpr);
    const int m = shadowMargin();

    const QRectF card = QRectF(rect()).adjusted(m + (m_side == LeftEdge ? kTail : 0), m + (m_side == Top ? kTail : 0),
                                                -(m + (m_side == RightEdge ? kTail : 0)),
                                                -(m + (m_side == Bottom ? kTail : 0)));
    Shadow::draw(&p, card, tipShadow(), dpr);

    QPainterPath shape;
    shape.addRoundedRect(card.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), pal.radiusCard, pal.radiusCard);
    if (m_side >= 0) {
        QPainterPath tail;
        const qreal t = m_tailPos;
        // the base sits slightly inside the card so the union has no seam
        switch (m_side) {
        case Top:
            tail.moveTo(t - kTailHalf, card.top() + 1);
            tail.lineTo(t, card.top() - kTail + bw);
            tail.lineTo(t + kTailHalf, card.top() + 1);
            break;
        case Bottom:
            tail.moveTo(t - kTailHalf, card.bottom() - 1);
            tail.lineTo(t, card.bottom() + kTail - bw);
            tail.lineTo(t + kTailHalf, card.bottom() - 1);
            break;
        case LeftEdge:
            tail.moveTo(card.left() + 1, t - kTailHalf);
            tail.lineTo(card.left() - kTail + bw, t);
            tail.lineTo(card.left() + 1, t + kTailHalf);
            break;
        default:
            tail.moveTo(card.right() - 1, t - kTailHalf);
            tail.lineTo(card.right() + kTail - bw, t);
            tail.lineTo(card.right() - 1, t + kTailHalf);
            break;
        }
        tail.closeSubpath();
        shape = shape.united(tail);
    }
    p.setBrush(pal.cardBg);
    p.setPen(QPen(pal.cardStroke, bw));
    p.drawPath(shape);
}

void TeachingTip::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    emit closed();
}

} // namespace QLight
