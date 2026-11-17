#pragma once

#include <QWidget>

#include "QLight/Export.h"

class QMenuBar;
namespace QWK { class WidgetWindowAgent; }

namespace QLight {

// Top-level window with a custom Fluent-style title bar (icon, title, minimize /
// maximize / close), built on QWindowKit so native behaviour is preserved:
// Aero Snap, resize borders, system menu, Windows 11 snap layouts.
//
// Only available when the library is configured with -DQLIGHT_WITH_WINDOWKIT=ON
// (which downloads QWindowKit at configure time). Windows is the tested platform.
//
// Put your content in setCentralWidget(); the window title and icon come from
// setWindowTitle() / setWindowIcon().
class QLIGHT_EXPORT FramelessWindow : public QWidget {
    Q_OBJECT
public:
    enum class Backdrop { None, Mica, MicaAlt, Acrylic };
    Q_ENUM(Backdrop)

    explicit FramelessWindow(QWidget *parent = nullptr);
    ~FramelessWindow() override;

    void setCentralWidget(QWidget *widget);
    QWidget *centralWidget() const { return m_central; }

    // Puts a menu bar into the title bar, between the icon and the title (the title is then centred).
    // The window takes ownership; a QLight::MenuBar is switched to its compact height. Pass nullptr to
    // remove it. Not used on macOS (native menu bar).
    void setMenuBar(QMenuBar *bar);
    QMenuBar *menuBar() const { return m_menuBar; }

    // System backdrop material (Windows 11 for Mica, Windows 10+ for Acrylic).
    // Returns false and keeps the solid background when the platform refuses.
    bool setBackdrop(Backdrop backdrop);
    Backdrop backdrop() const { return m_backdrop; }

protected:
    void paintEvent(QPaintEvent *) override;
    bool event(QEvent *e) override;

private:
    class TitleBar;

    void applyThemeToNativeFrame();

    QWK::WidgetWindowAgent *m_agent;
    TitleBar *m_titleBar;
    QWidget *m_central = nullptr;
    QMenuBar *m_menuBar = nullptr;
    Backdrop m_backdrop = Backdrop::None;
};

} // namespace QLight
