#pragma once

#include <QMenuBar>
#include <QStatusBar>

#include "QLight/Export.h"

namespace QLight {

class Menu;

// Flat menu bar: items highlight on hover / while their menu is open, no native frame. Inherits
// QMenuBar, so addMenu(), actions, Alt / mnemonic navigation and signals are unchanged. Use
// createMenu() (or addMenu() with your own QLight::Menu) so the drop-downs are Fluent menus too.
// The native macOS menu bar is not used (setNativeMenuBar(false)).
class QLIGHT_EXPORT MenuBar : public QMenuBar {
    Q_OBJECT
public:
    explicit MenuBar(QWidget *parent = nullptr);

    // Adds a QLight::Menu with the given title and returns it.
    Menu *createMenu(const QString &title);

    // 32px instead of 36px tall: used when the bar lives in a title bar (FramelessWindow::setMenuBar
    // switches it on).
    bool isCompact() const { return m_compact; }
    void setCompact(bool compact);

protected:
    void paintEvent(QPaintEvent *) override;
    bool event(QEvent *e) override;

private:
    void init();
    void styleChildren();

    bool m_compact = false;
};

// Status bar with a hairline on top and plain text messages; no size grip or item frames.
// Inherits QStatusBar: showMessage(), addWidget() / addPermanentWidget() work as usual.
class QLIGHT_EXPORT StatusBar : public QStatusBar {
    Q_OBJECT
public:
    explicit StatusBar(QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void init();
};

} // namespace QLight
