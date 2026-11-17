#pragma once

#include <QTabBar>
#include <QVariantAnimation>

#include "QLight/Export.h"

namespace QLight {

// Fluent-style tab strip (horizontal only). Inherits QTabBar, so the tab model,
// currentChanged, movable tabs, keyboard navigation and QStackedWidget wiring work as usual.
//
//  * Style::Tabs  - "TabView": the selected tab is a raised card-coloured tab that is meant
//                   to sit directly on a card-coloured content area; optional close buttons
//                   and a "+" button.
//  * Style::Pivot - text-only headers with an accent underline under the selected one.
//
// Close buttons and the add button are painted and hit-tested here (no child widgets):
// closing only emits tabCloseRequested(); it is up to you to remove the tab.
class QLIGHT_EXPORT TabBar : public QTabBar {
    Q_OBJECT
    Q_PROPERTY(Style barStyle READ barStyle WRITE setBarStyle)
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable)
    Q_PROPERTY(bool addButtonVisible READ isAddButtonVisible WRITE setAddButtonVisible)
public:
    enum Style { Tabs, Pivot };
    Q_ENUM(Style)

    explicit TabBar(QWidget *parent = nullptr);

    // (named barStyle rather than style so QWidget::style()/setStyle(QStyle*) stay reachable)
    Style barStyle() const { return m_style; }
    void setBarStyle(Style s);

    bool isClosable() const { return m_closable; }
    void setClosable(bool on);

    bool isAddButtonVisible() const { return m_addVisible; }
    void setAddButtonVisible(bool on);

    // Geometry of the painted buttons (empty when not shown); handy for tests and custom hit testing.
    QRect closeButtonRect(int index) const;
    QRect addButtonRect() const;

    QSize tabSizeHint(int index) const override;
    QSize minimumTabSizeHint(int index) const override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void addTabRequested();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    enum class Hit { None, Tab, Close, Add };
    struct HitResult { Hit kind = Hit::None; int index = -1; };

    void init();
    int barHeight() const;
    HitResult hitTest(const QPoint &pos) const;
    bool showsCloseButton(int index) const;

    Style m_style = Tabs;
    bool m_closable = false;
    bool m_addVisible = false;
    bool m_focusVisible = false;
    HitResult m_hover;
    HitResult m_pressed;
    QVariantAnimation m_pillAnim;  // Pivot underline
    QRectF m_pill;
    void animatePillToCurrent();
    QRectF pillTarget() const;
};

} // namespace QLight
