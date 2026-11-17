#include "QLight/SpinBox.h"

#include "QLight/Calendar.h"
#include "QLight/Flyout.h"
#include "QLight/FluentIcon.h"
#include "QLight/Theme.h"

#include <QAbstractSpinBox>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QStyleOptionSpinBox>
#include <QTimer>

#include <functional>

#include "ColorAnimator.h"
#include "Field.h"
#include "MetricStyle.h"

namespace QLight {

namespace {

constexpr int kHeight = 32;
constexpr int kButtonWidth = 28;
constexpr int kButtonInset = 2;
constexpr int kTextLeft = 8;   // QLineEdit adds ~2px of its own
constexpr int kMinTextRoom = 24;

// Geometry of the buttons and the edit field, shared by the style (hit testing, layout) and the
// painting code so they can never disagree. In calendar mode there is a single button instead
// of the up / down pair.
struct SpinGeometry {
    QRect up, down, button, edit;
};

SpinGeometry geometryFor(const QRect &r, bool calendar)
{
    SpinGeometry g;
    const int bh = r.height() - 2 * kButtonInset;
    const QRect last(r.right() - kButtonInset - kButtonWidth + 1, r.top() + kButtonInset, kButtonWidth, bh);
    if (calendar) {
        g.button = last;
        g.edit = QRect(r.left() + kTextLeft, r.top() + 1, last.left() - r.left() - kTextLeft - 2, r.height() - 2);
    } else {
        g.down = last;
        g.up = last.translated(-kButtonWidth, 0);
        g.edit = QRect(r.left() + kTextLeft, r.top() + 1, g.up.left() - r.left() - kTextLeft - 2, r.height() - 2);
    }
    return g;
}

class SpinStyle : public detail::MetricStyle {
public:
    explicit SpinStyle(bool calendar) : m_calendar(calendar) {}

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                         const QWidget *w) const override
    {
        if (cc != CC_SpinBox)
            return detail::MetricStyle::subControlRect(cc, opt, sc, w);
        const SpinGeometry g = geometryFor(opt->rect, m_calendar);
        switch (sc) {
        case SC_SpinBoxUp: return g.up;
        case SC_SpinBoxDown: return g.down;
        case SC_SpinBoxEditField: return g.edit;
        case SC_SpinBoxFrame: return opt->rect;
        default: return {};
        }
    }

    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pos,
                                     const QWidget *w) const override
    {
        if (cc != CC_SpinBox)
            return detail::MetricStyle::hitTestComplexControl(cc, opt, pos, w);
        const SpinGeometry g = geometryFor(opt->rect, m_calendar);
        if (g.up.contains(pos)) return SC_SpinBoxUp;
        if (g.down.contains(pos)) return SC_SpinBoxDown;
        if (g.edit.contains(pos)) return SC_SpinBoxEditField;
        return opt->rect.contains(pos) ? SC_SpinBoxFrame : SC_None; // the calendar button must not step
    }

    QSize sizeFromContents(ContentsType type, const QStyleOption *opt, const QSize &csz,
                           const QWidget *w) const override
    {
        if (type != CT_SpinBox)
            return detail::MetricStyle::sizeFromContents(type, opt, csz, w);
        const int buttons = (m_calendar ? 1 : 2) * kButtonWidth;
        return {qMax(csz.width(), kMinTextRoom) + kTextLeft + buttons + 2 * kButtonInset + 6, kHeight};
    }

private:
    bool m_calendar;
};

} // namespace

namespace detail {

// Attached to a QAbstractSpinBox: installs the geometry style, keeps the palette and fill
// state up to date, tracks hover / pressed buttons and paints the whole control.
class SpinSkin : public QObject {
public:
    enum class Mode { UpDown, Calendar };

    SpinSkin(QAbstractSpinBox *box, QLineEdit *edit, std::function<QAbstractSpinBox::StepEnabled()> steps,
             Mode mode = Mode::UpDown, std::function<void()> onButton = {})
        : QObject(box), m_box(box), m_edit(edit), m_steps(std::move(steps)), m_onButton(std::move(onButton)),
          m_fill(new ColorAnimator(this))
    {
        box->setFrame(false);
        box->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        box->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        box->setFocusPolicy(Qt::StrongFocus);
        box->setAttribute(Qt::WA_Hover);
        box->setMouseTracking(true);
        setMode(mode);

        edit->setFrame(false);
        edit->setTextMargins(0, 0, 0, 0);
        edit->setMouseTracking(true);
        applyFieldPalette(edit);
        applyFieldPalette(box);
        edit->installEventFilter(this);
        box->installEventFilter(this);

        connect(m_fill, &ColorAnimator::updated, box, qOverload<>(&QWidget::update));
        connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] {
            applyFieldPalette(m_edit);
            applyFieldPalette(m_box);
            syncFill(false);
            m_box->update();
        });
        syncFill(false);
    }

    // Switches between the up / down buttons and the single calendar button.
    void setMode(Mode mode)
    {
        m_mode = mode;
        auto *st = new SpinStyle(mode == Mode::Calendar);
        st->setParent(this);
        m_box->setStyle(st);
        delete m_style; // only after the box has switched over to the new one
        m_style = st;
        m_box->updateGeometry();
        m_box->update();
    }

    void paint(QPainter &p)
    {
        const Palette &pal = ThemeManager::instance().palette();
        const bool enabled = m_box->isEnabled();
        const qreal dpr = m_box->devicePixelRatioF();
        paintFieldFrame(p, QRectF(m_box->rect()), dpr, m_fill->color(), focused(), enabled);

        const SpinGeometry g = geometryFor(m_box->rect(), m_mode == Mode::Calendar);
        struct Btn { QRect r; Icon icon; bool on; Hit hit; };
        std::vector<Btn> buttons;
        if (m_mode == Mode::Calendar) {
            buttons.push_back({g.button, Icon::Calendar, true, Cal});
        } else {
            const auto steps = m_steps();
            buttons.push_back({g.up, Icon::ChevronUp, bool(steps & QAbstractSpinBox::StepUpEnabled), Up});
            buttons.push_back({g.down, Icon::ChevronDown, bool(steps & QAbstractSpinBox::StepDownEnabled), Down});
        }
        for (const Btn &b : buttons) {
            const bool usable = enabled && b.on;
            if (usable && (m_hover == b.hit || m_pressed == b.hit)) {
                p.setPen(Qt::NoPen);
                p.setBrush(m_pressed == b.hit ? pal.subtleFillPressed : pal.subtleFillHover);
                p.setRenderHint(QPainter::Antialiasing);
                p.drawRoundedRect(b.r, pal.radiusControl, pal.radiusControl);
            }
            const int s = b.icon == Icon::Calendar ? 16 : 12;
            const QColor c = usable ? (b.icon == Icon::Calendar ? pal.textPrimary : pal.textSecondary) : pal.textDisabled;
            p.drawPixmap(b.r.center() - QPoint(s / 2, s / 2), FluentIcon(b.icon).pixmap(s, c, dpr));
        }
    }

protected:
    bool eventFilter(QObject *obj, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            if (obj == m_edit || obj == m_box)
                syncFill(true);
            break;
        case QEvent::Enter:
        case QEvent::HoverEnter:
        case QEvent::Leave:
        case QEvent::HoverLeave:
            if (e->type() == QEvent::Leave || e->type() == QEvent::HoverLeave)
                setHover(None);
            syncFill(true);
            break;
        case QEvent::EnabledChange:
            syncFill(false);
            break;
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease: {
            auto *me = static_cast<QMouseEvent *>(e);
            const QPoint pos = m_box->mapFromGlobal(me->globalPosition().toPoint());
            if (e->type() == QEvent::MouseMove || e->type() == QEvent::MouseButtonPress)
                setHover(hitAt(pos));
            if (e->type() == QEvent::MouseButtonPress && me->button() == Qt::LeftButton) {
                setPressed(hitAt(pos));
            } else if (e->type() == QEvent::MouseButtonRelease) {
                const Hit was = m_pressed;
                setPressed(None);
                if (was == Cal && hitAt(pos) == Cal && me->button() == Qt::LeftButton && m_onButton && m_box->isEnabled())
                    QTimer::singleShot(0, m_box, [cb = m_onButton] { cb(); }); // after the click has been delivered
            }
            break;
        }
        default:
            break;
        }
        return false;
    }

private:
    enum Hit { None, Up, Down, Cal };

    bool focused() const { return m_edit->hasFocus() || m_box->hasFocus(); }

    Hit hitAt(const QPoint &pos) const
    {
        const SpinGeometry g = geometryFor(m_box->rect(), m_mode == Mode::Calendar);
        if (g.up.contains(pos)) return Up;
        if (g.down.contains(pos)) return Down;
        if (g.button.contains(pos)) return Cal;
        return None;
    }
    void setHover(Hit h)
    {
        if (h != m_hover) {
            m_hover = h;
            m_box->update();
        }
    }
    void setPressed(Hit h)
    {
        if (h != m_pressed) {
            m_pressed = h;
            m_box->update();
        }
    }
    void syncFill(bool animate)
    {
        m_fill->setTarget(fieldFill(m_box->isEnabled(), focused(), m_box->underMouse()), animate);
    }

    QAbstractSpinBox *m_box;
    QLineEdit *m_edit;
    std::function<QAbstractSpinBox::StepEnabled()> m_steps;
    std::function<void()> m_onButton;
    ColorAnimator *m_fill;
    SpinStyle *m_style = nullptr;
    Mode m_mode = Mode::UpDown;
    Hit m_hover = None;
    Hit m_pressed = None;
};

} // namespace detail

SpinBox::SpinBox(QWidget *parent)
    : QSpinBox(parent), m_skin(new detail::SpinSkin(this, lineEdit(), [this] { return stepEnabled(); }))
{
}

void SpinBox::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    m_skin->paint(p);
}

DoubleSpinBox::DoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent), m_skin(new detail::SpinSkin(this, lineEdit(), [this] { return stepEnabled(); }))
{
}

void DoubleSpinBox::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    m_skin->paint(p);
}

// ---------------------------------------------------------------------------
// Date / time

namespace {

// Opens a CalendarWidget in a Flyout below `edit`; picking a day sets the date and closes it.
// (QDateTimeEdit::setDate keeps the time of day, so this serves DateTimeEdit as well.)
void openCalendarFlyout(QDateTimeEdit *edit)
{
    auto *cal = new CalendarWidget;
    cal->setDateRange(edit->minimumDate(), edit->maximumDate());
    cal->setSelectedDate(edit->date());
    cal->setCurrentPage(edit->date().year(), edit->date().month());

    auto *flyout = new Flyout(cal, edit->window());
    flyout->setAttribute(Qt::WA_DeleteOnClose);
    flyout->setPadding(8);
    QObject::connect(cal, &CalendarWidget::clicked, flyout, [edit = QPointer<QDateTimeEdit>(edit), flyout](const QDate &d) {
        if (edit)
            edit->setDate(d);
        flyout->close();
    });
    QObject::connect(cal, &CalendarWidget::activated, flyout, [edit = QPointer<QDateTimeEdit>(edit), flyout](const QDate &d) {
        if (edit)
            edit->setDate(d);
        flyout->close();
    });
    flyout->popup(edit, Flyout::Placement::Auto);
    cal->setFocus(Qt::PopupFocusReason);
}

} // namespace

DateEdit::DateEdit(QWidget *parent) : QDateEdit(parent) { init(); }
DateEdit::DateEdit(QDate date, QWidget *parent) : QDateEdit(date, parent) { init(); }

void DateEdit::init()
{
    m_skin = new detail::SpinSkin(this, lineEdit(), [this] { return stepEnabled(); },
                                  detail::SpinSkin::Mode::Calendar, [this] { showCalendar(); });
}

void DateEdit::setCalendarFlyoutEnabled(bool on)
{
    m_flyout = on;
    m_skin->setMode(on ? detail::SpinSkin::Mode::Calendar : detail::SpinSkin::Mode::UpDown);
}

void DateEdit::showCalendar() { openCalendarFlyout(this); }

void DateEdit::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    m_skin->paint(p);
}

DateTimeEdit::DateTimeEdit(QWidget *parent) : QDateTimeEdit(parent) { init(); }
DateTimeEdit::DateTimeEdit(const QDateTime &dt, QWidget *parent) : QDateTimeEdit(dt, parent) { init(); }

void DateTimeEdit::init()
{
    m_skin = new detail::SpinSkin(this, lineEdit(), [this] { return stepEnabled(); },
                                  detail::SpinSkin::Mode::Calendar, [this] { showCalendar(); });
}

void DateTimeEdit::setCalendarFlyoutEnabled(bool on)
{
    m_flyout = on;
    m_skin->setMode(on ? detail::SpinSkin::Mode::Calendar : detail::SpinSkin::Mode::UpDown);
}

void DateTimeEdit::showCalendar() { openCalendarFlyout(this); }

void DateTimeEdit::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    m_skin->paint(p);
}

TimeEdit::TimeEdit(QWidget *parent) : QTimeEdit(parent) { init(); }
TimeEdit::TimeEdit(QTime time, QWidget *parent) : QTimeEdit(time, parent) { init(); }

void TimeEdit::init() { m_skin = new detail::SpinSkin(this, lineEdit(), [this] { return stepEnabled(); }); }

void TimeEdit::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    m_skin->paint(p);
}

} // namespace QLight
