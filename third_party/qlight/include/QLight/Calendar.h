#pragma once

#include <QDate>
#include <QWidget>

#include "QLight/Export.h"

namespace QLight {

// Month calendar with a Fluent look: accent circle on the selected day, a ring around today, hover
// fill, dimmed days of neighbouring months, previous / next month buttons. Painted and hit-tested
// by the widget itself.
//
// It follows QCalendarWidget's most used API (selectedDate, date range, shown month, selectionChanged /
// clicked / activated) but is a plain QWidget: no week numbers, no grid / header format options,
// no date text formats. Keyboard: arrows move by day / week, PageUp / PageDown by month
// (Ctrl: by year), Home / End jump to the first / last day of the month, Enter activates.
class QLIGHT_EXPORT CalendarWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QDate selectedDate READ selectedDate WRITE setSelectedDate NOTIFY selectionChanged)
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate)
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate)
public:
    explicit CalendarWidget(QWidget *parent = nullptr);

    QDate selectedDate() const { return m_selected; }
    void setSelectedDate(const QDate &date); // ignored when outside the range; also shows its month

    QDate minimumDate() const { return m_min; }
    void setMinimumDate(const QDate &date);
    QDate maximumDate() const { return m_max; }
    void setMaximumDate(const QDate &date);
    void setDateRange(const QDate &min, const QDate &max);

    int currentPageYear() const { return m_year; }
    int currentPageMonth() const { return m_month; }
    void setCurrentPage(int year, int month);

    Qt::DayOfWeek firstDayOfWeek() const { return m_firstDow; }
    void setFirstDayOfWeek(Qt::DayOfWeek dow);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }

    // Geometry helpers (widget coordinates), mainly for tests: the cell of a date in the shown
    // month grid (empty if the date is not visible), and the two month buttons.
    QRect dateRect(const QDate &date) const;
    QRect previousButtonRect() const;
    QRect nextButtonRect() const;

public slots:
    void showNextMonth();
    void showPreviousMonth();
    void showNextYear();
    void showPreviousYear();

signals:
    void selectionChanged();
    void clicked(const QDate &date);   // a day was clicked
    void activated(const QDate &date); // double click or Enter on the selected day
    void currentPageChanged(int year, int month);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

private:
    enum class Hit { None, Prev, Next, Day };
    struct HitResult {
        Hit kind = Hit::None;
        QDate date;
        bool operator==(const HitResult &o) const { return kind == o.kind && date == o.date; }
    };

    QDate firstVisible() const;
    bool inRange(const QDate &d) const { return d >= m_min && d <= m_max; }
    HitResult hitTest(const QPoint &pos) const;
    void moveSelection(const QDate &target);

    QDate m_selected;
    QDate m_min{1752, 9, 14};
    QDate m_max{9999, 12, 31};
    int m_year = 2000;
    int m_month = 1;
    Qt::DayOfWeek m_firstDow = Qt::Monday;
    HitResult m_hover, m_pressed;
    bool m_focusVisible = false;
};

} // namespace QLight
