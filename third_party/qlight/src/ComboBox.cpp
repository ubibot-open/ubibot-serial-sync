#include "QLight/ComboBox.h"

#include "QLight/ScrollBar.h"
#include "QLight/Shadow.h"
#include "QLight/Theme.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QScreen>
#include <QStyledItemDelegate>

#include "ColorAnimator.h"
#include "Util.h"

namespace QLight {

namespace {
constexpr int kHeight = 32;
constexpr int kMinWidth = 120;
constexpr int kPaddingLeft = 12;
constexpr int kChevronArea = 32;
constexpr int kIconSize = 16;
constexpr int kIconGap = 8;

constexpr int kItemHeight = 38;  // row height; visible pill is 6px shorter
constexpr int kMaxRows = 8;
constexpr int kPopupPadX = 4;
constexpr int kPopupPadY = 2;
constexpr int kPopupGap = 4;     // between combo and flyout
constexpr int kFadeMs = 120;

ShadowSpec flyoutShadow()
{
    ShadowSpec s;
    s.radius = ThemeManager::instance().palette().radiusCard;
    s.blur = 12;
    s.offsetY = 4;
    s.color = ThemeManager::instance().palette().shadowColor;
    s.color.setAlpha(qMin(255, s.color.alpha() * 3 / 2));
    return s;
}
} // namespace

namespace detail {

class ItemDelegate : public QStyledItemDelegate {
public:
    ItemDelegate(const ComboBox *combo, QObject *parent) : QStyledItemDelegate(parent), m_combo(combo) {}

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        return {0, kItemHeight};
    }

    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const override
    {
        const Palette &pal = ThemeManager::instance().palette();
        const bool enabled = index.flags() & Qt::ItemIsEnabled;
        const QRectF inner = QRectF(opt.rect).adjusted(0, 3, 0, -3);

        p->save();
        p->setRenderHint(QPainter::Antialiasing);

        if (enabled && (opt.state & QStyle::State_Selected)) {
            p->setPen(Qt::NoPen);
            p->setBrush(pal.subtleFillHover);
            p->drawRoundedRect(inner, pal.radiusControl, pal.radiusControl);
        }
        if (index.row() == m_combo->currentIndex()) { // accent pill marks the current item
            p->setPen(Qt::NoPen);
            p->setBrush(enabled ? pal.accent : pal.accentDisabled);
            p->drawRoundedRect(QRectF(inner.left() + 1, inner.center().y() - 8, 3, 16), 1.5, 1.5);
        }

        qreal x = inner.left() + kPaddingLeft;
        const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        if (!icon.isNull()) {
            icon.paint(p, QRect(qRound(x), qRound(inner.center().y() - kIconSize / 2.0), kIconSize, kIconSize),
                       Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);
            x += kIconSize + kIconGap;
        }
        p->setPen(enabled ? pal.textPrimary : pal.textDisabled);
        p->setFont(opt.font);
        const QString text = opt.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(),
                                                        Qt::ElideRight, qRound(inner.right() - x - 8));
        p->drawText(QRectF(x, inner.top(), inner.right() - x, inner.height()),
                    Qt::AlignVCenter | Qt::AlignLeft, text);
        p->restore();
    }

private:
    const ComboBox *m_combo;
};

class ComboPopup : public QWidget {
    Q_OBJECT
public:
    explicit ComboPopup(ComboBox *combo)
        : QWidget(combo, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_combo(combo), m_view(new QListView(this)),
          m_fade(new QPropertyAnimation(this, "windowOpacity", this))
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setFocusProxy(m_view);

        m_view->setItemDelegate(new ItemDelegate(combo, m_view));
        m_view->setFrameShape(QFrame::NoFrame);
        m_view->setUniformItemSizes(true);
        m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_view->setSelectionMode(QAbstractItemView::SingleSelection);
        m_view->setMouseTracking(true);
        m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_view->setVerticalScrollBar(new ScrollBar(Qt::Vertical, m_view));
        m_view->viewport()->setAutoFillBackground(false);
        QPalette pal = m_view->palette();
        pal.setColor(QPalette::Base, Qt::transparent);
        m_view->setPalette(pal);

        // one highlight only: hovering moves the current row, like native combos
        connect(m_view, &QListView::entered, this, [this](const QModelIndex &i) {
            if (i.flags() & Qt::ItemIsEnabled)
                m_view->setCurrentIndex(i);
        });
        connect(m_view, &QListView::clicked, this, &ComboPopup::choose);
        connect(m_view, &QListView::activated, this, &ComboPopup::choose); // Enter/Return

        m_fade->setDuration(kFadeMs);
        m_fade->setStartValue(0.0);
        m_fade->setEndValue(1.0);

        connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
                qOverload<>(&QWidget::update));
    }

    void popup()
    {
        m_view->setModel(m_combo->model());
        m_view->setRootIndex(m_combo->rootModelIndex());
        m_view->setModelColumn(m_combo->modelColumn());
        m_view->setFont(m_combo->font());
        setFont(m_combo->font());

        const int rows = qMax(1, qMin(m_combo->count(), kMaxRows));
        const QSize card(m_combo->width(), rows * kItemHeight + 2 * kPopupPadY);
        const int m = margin();
        setFixedSize(card.width() + 2 * m, card.height() + 2 * m);
        m_view->setGeometry(m + kPopupPadX, m + kPopupPadY, card.width() - 2 * kPopupPadX,
                            rows * kItemHeight);

        const QPoint below = m_combo->mapToGlobal(QPoint(0, m_combo->height() + kPopupGap));
        const QRect avail = m_combo->screen()->availableGeometry();
        int y = below.y();
        if (y + card.height() > avail.bottom())
            y = m_combo->mapToGlobal(QPoint(0, 0)).y() - kPopupGap - card.height();
        const int x = qBound(avail.left(), below.x(), qMax(avail.left(), avail.right() - card.width()));
        move(x - m, y - m);

        const QModelIndex cur = m_combo->model()->index(m_combo->currentIndex(), m_combo->modelColumn(),
                                                        m_combo->rootModelIndex());
        m_view->setCurrentIndex(cur);
        m_view->scrollTo(cur, QAbstractItemView::PositionAtCenter);

        setWindowOpacity(0.0);
        show();
        m_fade->start();
        m_view->setFocus();
    }

    int margin() const { return int(Shadow::extent(flyoutShadow())); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        const Palette &pal = ThemeManager::instance().palette();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const int m = margin();
        const QRectF body = QRectF(rect()).adjusted(m, m, -m, -m);
        const qreal dpr = devicePixelRatioF();

        Shadow::draw(&p, body, flyoutShadow(), dpr);
        const qreal bw = detail::hairline(dpr);
        p.setBrush(pal.cardBg);
        p.setPen(QPen(pal.cardStroke, bw));
        p.drawRoundedRect(body.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), pal.radiusCard, pal.radiusCard);
    }

    // For a Qt::Popup, clicks outside arrive here. Clicking the combo itself must
    // close the flyout without being replayed, or it would immediately reopen.
    void mousePressEvent(QMouseEvent *e) override
    {
        const QPoint local = m_combo->mapFromGlobal(e->globalPosition().toPoint());
        setAttribute(Qt::WA_NoMouseReplay, m_combo->rect().contains(local));
        QWidget::mousePressEvent(e);
        if (!rect().contains(e->position().toPoint()))
            close();
    }

    void hideEvent(QHideEvent *e) override
    {
        QWidget::hideEvent(e);
        m_fade->stop();
        m_combo->syncTarget(true);
        m_combo->update();
    }

private:
    void choose(const QModelIndex &i)
    {
        if (!isVisible() || !(i.flags() & Qt::ItemIsEnabled))
            return;
        m_combo->popupChosen(i.row());
    }

    ComboBox *m_combo;
    QListView *m_view;
    QPropertyAnimation *m_fade;
};

} // namespace detail

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent), m_fill(new detail::ColorAnimator(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(m_fill, &detail::ColorAnimator::updated, this, qOverload<>(&QWidget::update));
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            [this] { syncTarget(false); });
    syncTarget(false);
}

bool ComboBox::popupOpen() const { return m_popup && m_popup->isVisible(); }

void ComboBox::syncTarget(bool animate)
{
    const Palette &p = ThemeManager::instance().palette();
    QColor c;
    if (!isEnabled()) c = p.controlFillDisabled;
    else if (popupOpen()) c = p.controlFillPressed;
    else if (underMouse()) c = p.controlFillHover;
    else c = p.controlFill;
    m_fill->setTarget(c, animate);
}

QSize ComboBox::sizeHint() const
{
    const QFontMetrics fm(font());
    int textW = 0;
    bool hasIcon = false;
    for (int i = 0; i < count(); ++i) {
        textW = qMax(textW, fm.horizontalAdvance(itemText(i)));
        hasIcon = hasIcon || !itemIcon(i).isNull();
    }
    textW = qMax(textW, fm.horizontalAdvance(placeholderText()));
    const int w = kPaddingLeft + (hasIcon ? kIconSize + kIconGap : 0) + textW + kChevronArea;
    return {qMax(kMinWidth, w), kHeight};
}

QSize ComboBox::minimumSizeHint() const { return {kMinWidth, kHeight}; }

void ComboBox::showPopup()
{
    if (isEditable() || count() == 0) {
        QComboBox::showPopup();
        return;
    }
    if (!m_popup)
        m_popup = new detail::ComboPopup(this);
    m_popup->popup();
    syncTarget(true);
    update();
}

void ComboBox::hidePopup()
{
    if (m_popup && m_popup->isVisible())
        m_popup->hide();
    else
        QComboBox::hidePopup();
}

void ComboBox::popupChosen(int row)
{
    hidePopup();
    if (row != currentIndex())
        setCurrentIndex(row);
    emit activated(row);
    emit textActivated(itemText(row));
}

void ComboBox::paintEvent(QPaintEvent *e)
{
    if (isEditable()) {
        QComboBox::paintEvent(e);
        return;
    }
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal dpr = devicePixelRatioF();
    const qreal bw = detail::hairline(dpr);
    const QRectF r = QRectF(rect()).adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2);
    const qreal rad = pal.radiusControl;

    p.setPen(Qt::NoPen);
    p.setBrush(m_fill->color());
    p.drawRoundedRect(r, rad, rad);

    QColor bottom = isEnabled() ? pal.controlStrokeBottom : pal.controlStrokeTop;
    QLinearGradient g(r.topLeft(), r.bottomLeft());
    const qreal edge = 1.0 - 3.0 / qMax(1, height());
    g.setColorAt(0, pal.controlStrokeTop);
    g.setColorAt(edge, pal.controlStrokeTop);
    g.setColorAt(1, bottom);
    p.setPen(QPen(QBrush(g), bw));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, rad, rad);

    // current item (icon + text) or placeholder
    const int idx = currentIndex();
    qreal x = kPaddingLeft;
    if (idx >= 0) {
        const QIcon icon = itemIcon(idx);
        if (!icon.isNull()) {
            icon.paint(&p, QRect(qRound(x), (height() - kIconSize) / 2, kIconSize, kIconSize),
                       Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
            x += kIconSize + kIconGap;
        }
    }
    const QString text = idx >= 0 ? itemText(idx) : placeholderText();
    p.setPen(!isEnabled() ? pal.textDisabled : idx >= 0 ? pal.textPrimary : pal.textPlaceholder);
    p.setFont(font());
    const int avail = qRound(width() - kChevronArea - x);
    p.drawText(QRectF(x, 0, avail, height()), Qt::AlignVCenter | Qt::AlignLeft,
               fontMetrics().elidedText(text, Qt::ElideRight, avail));

    // chevron
    QPen pen(isEnabled() ? pal.textSecondary : pal.textDisabled, 1.4, Qt::SolidLine, Qt::RoundCap,
             Qt::RoundJoin);
    p.setPen(pen);
    const QPointF c(width() - kChevronArea / 2.0 - 2, height() / 2.0);
    QPainterPath chevron;
    chevron.moveTo(c + QPointF(-4, -1.8));
    chevron.lineTo(c + QPointF(0, 2.2));
    chevron.lineTo(c + QPointF(4, -1.8));
    p.drawPath(chevron);

    if (m_focusVisible && isEnabled()) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(pal.focusStroke, 2));
        p.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), rad + 1, rad + 1);
        p.setPen(QPen(pal.windowBg, 1));
        p.drawRoundedRect(QRectF(rect()).adjusted(2.5, 2.5, -2.5, -2.5), rad, rad);
    }
}

void ComboBox::mousePressEvent(QMouseEvent *e)
{
    if (isEditable() || e->button() != Qt::LeftButton) {
        QComboBox::mousePressEvent(e);
        return;
    }
    if (isEnabled() && rect().contains(e->position().toPoint()))
        showPopup();
    e->accept();
}

void ComboBox::mouseReleaseEvent(QMouseEvent *e)
{
    if (isEditable())
        QComboBox::mouseReleaseEvent(e);
    else
        e->accept();
}

void ComboBox::wheelEvent(QWheelEvent *e)
{
    // like modern Fluent apps: scrolling over a page must not change values by accident
    if (!hasFocus())
        e->ignore();
    else
        QComboBox::wheelEvent(e);
}

void ComboBox::enterEvent(QEnterEvent *e)
{
    QComboBox::enterEvent(e);
    syncTarget(true);
}

void ComboBox::leaveEvent(QEvent *e)
{
    QComboBox::leaveEvent(e);
    syncTarget(true);
}

void ComboBox::focusInEvent(QFocusEvent *e)
{
    QComboBox::focusInEvent(e);
    m_focusVisible = detail::isKeyboardFocusReason(e->reason());
    update();
}

void ComboBox::focusOutEvent(QFocusEvent *e)
{
    QComboBox::focusOutEvent(e);
    m_focusVisible = false;
    update();
}

void ComboBox::changeEvent(QEvent *e)
{
    QComboBox::changeEvent(e);
    if (e->type() == QEvent::EnabledChange)
        syncTarget(false);
    else if (e->type() == QEvent::FontChange)
        updateGeometry();
}

} // namespace QLight

#include "ComboBox.moc"
