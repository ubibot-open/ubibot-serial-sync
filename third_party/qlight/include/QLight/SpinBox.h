#pragma once

#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTimeEdit>

#include "QLight/Export.h"

namespace QLight {

namespace detail { class SpinSkin; }

// Fluent-style number entry with two chevron buttons at the right end (up, down).
// Inherit QSpinBox / QDoubleSpinBox, so range, step, prefix/suffix, wrapping,
// keyboard and wheel behaviour and all signals are unchanged.
class QLIGHT_EXPORT SpinBox : public QSpinBox {
    Q_OBJECT
public:
    explicit SpinBox(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    detail::SpinSkin *m_skin;
};

class QLIGHT_EXPORT DoubleSpinBox : public QDoubleSpinBox {
    Q_OBJECT
public:
    explicit DoubleSpinBox(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    detail::SpinSkin *m_skin;
};

// Date / time entry in the same field look. The sections are edited with the keyboard, the arrow
// keys and the wheel as in Qt; DateEdit and DateTimeEdit replace the up / down buttons by a
// calendar button that opens a QLight::CalendarWidget in a Flyout (turn that off with
// setCalendarFlyoutEnabled(false) to get the up / down buttons back). TimeEdit keeps up / down.
class QLIGHT_EXPORT DateEdit : public QDateEdit {
    Q_OBJECT
    Q_PROPERTY(bool calendarFlyoutEnabled READ isCalendarFlyoutEnabled WRITE setCalendarFlyoutEnabled)
public:
    explicit DateEdit(QWidget *parent = nullptr);
    explicit DateEdit(QDate date, QWidget *parent = nullptr);

    bool isCalendarFlyoutEnabled() const { return m_flyout; }
    void setCalendarFlyoutEnabled(bool on);

    // Opens (or closes) the calendar flyout programmatically.
    void showCalendar();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void init();
    detail::SpinSkin *m_skin = nullptr;
    bool m_flyout = true;
};

class QLIGHT_EXPORT DateTimeEdit : public QDateTimeEdit {
    Q_OBJECT
    Q_PROPERTY(bool calendarFlyoutEnabled READ isCalendarFlyoutEnabled WRITE setCalendarFlyoutEnabled)
public:
    explicit DateTimeEdit(QWidget *parent = nullptr);
    explicit DateTimeEdit(const QDateTime &dt, QWidget *parent = nullptr);

    bool isCalendarFlyoutEnabled() const { return m_flyout; }
    void setCalendarFlyoutEnabled(bool on);
    void showCalendar();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void init();
    detail::SpinSkin *m_skin = nullptr;
    bool m_flyout = true;
};

class QLIGHT_EXPORT TimeEdit : public QTimeEdit {
    Q_OBJECT
public:
    explicit TimeEdit(QWidget *parent = nullptr);
    explicit TimeEdit(QTime time, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void init();
    detail::SpinSkin *m_skin = nullptr;
};

} // namespace QLight
