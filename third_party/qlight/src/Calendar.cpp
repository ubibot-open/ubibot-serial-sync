#include "QLight/Calendar.h"

#include "QLight/FluentIcon.h"
#include "QLight/Theme.h"

#include <QFocusEvent>
#include <QFontMetricsF>
#include <QKeyEvent>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include "Util.h"

namespace QLight {

namespace {

constexpr int kCellW = 40;
constexpr int kCellH = 36;
constexpr int kHeaderH = 44;
constexpr int kWeekdayH = 28;
constexpr int kRows = 6;
constexpr int kCols = 7;
constexpr int kNavSize = 32;

} // namespace

CalendarWidget::CalendarWidget(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    m_firstDow = QLocale().firstDayOfWeek();
    m_selected = QDate::currentDate();
    m_year = m_selected.year();
    m_month = m_selected.month();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, qOverload<>(&QWidget::update));
}

QSize CalendarWidget::sizeHint() const { return {kCols * kCellW, kHeaderH + kWeekdayH + kRows * kCellH}; }

void CalendarWidget::setSelectedDate(const QDate &date)
{
    if (!date.isValid() || !inRange(date) || date == m_selected)
        return;
    m_selected = date;
    setCurrentPage(date.year(), date.month());
    update();
    emit selectionChanged();
}

void CalendarWidget::setMinimumDate(const QDate &date) { setDateRange(date, m_max); }
void CalendarWidget::setMaximumDate(const QDate &date) { setDateRange(m_min, date); }

void CalendarWidget::setDateRange(const QDate &min, const QDate &max)
{
    if (!min.isValid() || !max.isValid() || min > max)
        return;
    m_min = min;
    m_max = max;
    if (!inRange(m_selected)) {
        m_selected = qBound(m_min, m_selected, m_max);
        emit selectionChanged();
    }
    update();
}

void CalendarWidget::setCurrentPage(int year, int month)
{
    // keep the shown month inside the allowed range
    QDate page(year, month, 1);
    if (!page.isValid())
        return;
    const QDate lo(m_min.year(), m_min.month(), 1), hi(m_max.year(), m_max.month(), 1);
    page = qBound(lo, page, hi);
    if (page.year() == m_year && page.month() == m_month)
        return;
    m_year = page.year();
    m_month = page.month();
    update();
    emit currentPageChanged(m_year, m_month);
}

void CalendarWidget::setFirstDayOfWeek(Qt::DayOfWeek dow)
{
    m_firstDow = dow;
    update();
}

void CalendarWidget::showNextMonth() { const QDate d = QDate(m_year, m_month, 1).addMonths(1); setCurrentPage(d.year(), d.month()); }
void CalendarWidget::showPreviousMonth() { const QDate d = QDate(m_year, m_month, 1).addMonths(-1); setCurrentPage(d.year(), d.month()); }
void CalendarWidget::showNextYear() { setCurrentPage(m_year + 1, m_month); }
void CalendarWidget::showPreviousYear() { setCurrentPage(m_year - 1, m_month); }

QDate CalendarWidget::firstVisible() const
{
    const QDate first(m_year, m_month, 1);
    const int offset = (first.dayOfWeek() - int(m_firstDow) + 7) % 7;
    return first.addDays(-(offset == 0 ? 0 : offset));
}

QRect CalendarWidget::previousButtonRect() const { return QRect(width() - 2 * kNavSize - 4, (kHeaderH - kNavSize) / 2, kNavSize, kNavSize); }
QRect CalendarWidget::nextButtonRect() const { return QRect(width() - kNavSize - 4, (kHeaderH - kNavSize) / 2, kNavSize, kNavSize); }

QRect CalendarWidget::dateRect(const QDate &date) const
{
    const qint64 i = firstVisible().daysTo(date);
    if (i < 0 || i >= kRows * kCols)
        return {};
    const int cellW = qMax(1, width() / kCols);
    return QRect(int(i % kCols) * cellW + (width() - cellW * kCols) / 2, kHeaderH + kWeekdayH + int(i / kCols) * kCellH,
                 cellW, kCellH);
}

CalendarWidget::HitResult CalendarWidget::hitTest(const QPoint &pos) const
{
    if (previousButtonRect().contains(pos)) return {Hit::Prev, {}};
    if (nextButtonRect().contains(pos)) return {Hit::Next, {}};
    const QDate start = firstVisible();
    for (int i = 0; i < kRows * kCols; ++i) {
        const QDate d = start.addDays(i);
        if (dateRect(d).contains(pos))
            return {Hit::Day, d};
    }
    return {};
}

void CalendarWidget::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal dpr = devicePixelRatioF();
    p.setFont(font());

    // header: "Month year" and the two month buttons
    {
        QFont f = font();
        f.setWeight(QFont::DemiBold);
        p.setFont(f);
        p.setPen(pal.textPrimary);
        p.drawText(QRectF(8, 0, width() - 2 * kNavSize - 16, kHeaderH), Qt::AlignVCenter | Qt::AlignLeft,
                   QLocale().toString(QDate(m_year, m_month, 1), QStringLiteral("MMMM yyyy")));
        p.setFont(font());

        const QDate prevMonth = QDate(m_year, m_month, 1).addMonths(-1);
        const QDate nextMonth = QDate(m_year, m_month, 1).addMonths(1);
        const bool prevOk = QDate(prevMonth.year(), prevMonth.month(), prevMonth.daysInMonth()) >= m_min;
        const bool nextOk = nextMonth <= m_max;
        struct Nav { QRect r; Icon icon; bool ok; Hit hit; };
        for (const Nav &n : {Nav{previousButtonRect(), Icon::ChevronUp, prevOk, Hit::Prev},
                             Nav{nextButtonRect(), Icon::ChevronDown, nextOk, Hit::Next}}) {
            const bool hover = m_hover.kind == n.hit && n.ok;
            const bool down = m_pressed.kind == n.hit && hover;
            if (hover) {
                p.setPen(Qt::NoPen);
                p.setBrush(down ? pal.subtleFillPressed : pal.subtleFillHover);
                p.drawRoundedRect(n.r, pal.radiusControl, pal.radiusControl);
            }
            p.drawPixmap(n.r.center() - QPoint(6, 6), FluentIcon(n.icon).pixmap(12, n.ok ? pal.textPrimary : pal.textDisabled, dpr));
        }
    }

    // weekday names
    const int cellW = qMax(1, width() / kCols);
    const int gridLeft = (width() - cellW * kCols) / 2;
    p.setPen(pal.textSecondary);
    for (int c = 0; c < kCols; ++c) {
        const int dow = (int(m_firstDow) - 1 + c) % 7 + 1;
        p.drawText(QRect(gridLeft + c * cellW, kHeaderH, cellW, kWeekdayH), Qt::AlignCenter,
                   QLocale().standaloneDayName(dow, QLocale::ShortFormat).left(2));
    }

    // days
    const QDate today = QDate::currentDate();
    const QDate start = firstVisible();
    for (int i = 0; i < kRows * kCols; ++i) {
        const QDate d = start.addDays(i);
        const QRect r = dateRect(d);
        const bool inMonth = d.month() == m_month;
        const bool enabled = inRange(d);
        const bool selected = d == m_selected;
        const bool hovered = m_hover.kind == Hit::Day && m_hover.date == d && enabled;
        const QPointF c = QRectF(r).center();
        const qreal rad = qMin(r.width(), r.height()) / 2.0 - 2;

        if (selected) {
            p.setPen(Qt::NoPen);
            p.setBrush(enabled ? pal.accent : pal.accentDisabled);
            p.drawEllipse(c, rad, rad);
        } else if (hovered) {
            p.setPen(Qt::NoPen);
            p.setBrush(m_pressed.kind == Hit::Day && m_pressed.date == d ? pal.subtleFillPressed : pal.subtleFillHover);
            p.drawEllipse(c, rad, rad);
        }
        if (d == today && !selected) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(pal.accent, 1.5));
            p.drawEllipse(c, rad - 0.5, rad - 0.5);
        }

        QColor text = selected ? pal.textOnAccent : !enabled ? pal.textDisabled : inMonth ? pal.textPrimary : pal.textSecondary;
        if (!inMonth && !selected && enabled)
            text.setAlpha(text.alpha() * 7 / 10);
        p.setPen(text);
        p.drawText(r, Qt::AlignCenter, QString::number(d.day()));

        if (m_focusVisible && selected) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(pal.focusStroke, 2));
            p.drawEllipse(c, rad + 1.5, rad + 1.5);
        }
    }
}

void CalendarWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    m_pressed = hitTest(e->position().toPoint());
    update();
    e->accept();
}

void CalendarWidget::mouseMoveEvent(QMouseEvent *e)
{
    const HitResult h = hitTest(e->position().toPoint());
    if (!(h == m_hover)) {
        m_hover = h;
        update();
    }
}

void CalendarWidget::mouseReleaseEvent(QMouseEvent *e)
{
    const HitResult pressed = m_pressed;
    m_pressed = {};
    const HitResult h = hitTest(e->position().toPoint());
    update();
    if (e->button() != Qt::LeftButton || !(h == pressed))
        return;
    if (h.kind == Hit::Prev) {
        showPreviousMonth();
    } else if (h.kind == Hit::Next) {
        showNextMonth();
    } else if (h.kind == Hit::Day && inRange(h.date)) {
        setSelectedDate(h.date);
        emit clicked(h.date);
    }
}

void CalendarWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    const HitResult h = hitTest(e->position().toPoint());
    if (e->button() == Qt::LeftButton && h.kind == Hit::Day && inRange(h.date)) {
        setSelectedDate(h.date);
        emit activated(h.date);
    }
}

void CalendarWidget::leaveEvent(QEvent *e)
{
    QWidget::leaveEvent(e);
    m_hover = {};
    update();
}

void CalendarWidget::moveSelection(const QDate &target)
{
    if (target.isValid() && inRange(target))
        setSelectedDate(target);
}

void CalendarWidget::keyPressEvent(QKeyEvent *e)
{
    const bool ctrl = e->modifiers() & Qt::ControlModifier;
    switch (e->key()) {
    case Qt::Key_Left: moveSelection(m_selected.addDays(-1)); break;
    case Qt::Key_Right: moveSelection(m_selected.addDays(1)); break;
    case Qt::Key_Up: moveSelection(m_selected.addDays(-7)); break;
    case Qt::Key_Down: moveSelection(m_selected.addDays(7)); break;
    case Qt::Key_PageUp: moveSelection(m_selected.addMonths(ctrl ? -12 : -1)); break;
    case Qt::Key_PageDown: moveSelection(m_selected.addMonths(ctrl ? 12 : 1)); break;
    case Qt::Key_Home: moveSelection(QDate(m_selected.year(), m_selected.month(), 1)); break;
    case Qt::Key_End: moveSelection(QDate(m_selected.year(), m_selected.month(), m_selected.daysInMonth())); break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        emit activated(m_selected);
        break;
    default:
        QWidget::keyPressEvent(e);
        return;
    }
    m_focusVisible = true;
    update();
}

void CalendarWidget::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() > 0)
        showPreviousMonth();
    else if (e->angleDelta().y() < 0)
        showNextMonth();
    e->accept();
}

void CalendarWidget::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
    m_focusVisible = detail::isKeyboardFocusReason(e->reason());
    update();
}

void CalendarWidget::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
    m_focusVisible = false;
    update();
}

} // namespace QLight
