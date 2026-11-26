#include "QLight/FramelessWindow.h"

#include "QLight/MenuBar.h"
#include "QLight/Theme.h"

#include <QAbstractButton>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QTimer>
#include <QVBoxLayout>

#include <QWKWidgets/widgetwindowagent.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "ColorAnimator.h"
#include "Util.h"

namespace QLight {

namespace {

constexpr int kBarHeight = 32;
constexpr int kButtonWidth = 46;
constexpr int kIconSize = 16;
[[maybe_unused]] constexpr int kMacButtonArea = 75; // close / minimize / zoom

const QColor kCloseHover(0xC4, 0x2B, 0x1C);
const QColor kClosePressed(0xC4, 0x2B, 0x1C, 0xCC);

} // namespace

// Caption button: minimize / maximize(restore) / close, glyphs drawn by us
// (Segoe Fluent Icons cannot be redistributed).
class CaptionButton : public QAbstractButton {
public:
    enum Kind { Minimize, Maximize, Close };

    CaptionButton(Kind kind, QWidget *parent)
        : QAbstractButton(parent), m_kind(kind), m_fill(new detail::ColorAnimator(this, 100)),
          m_glyph(new detail::ColorAnimator(this, 100))
    {
        setFocusPolicy(Qt::NoFocus);
        setFixedSize(kButtonWidth, kBarHeight);
        setAccessibleName(kind == Minimize ? tr("Minimize") : kind == Maximize ? tr("Maximize") : tr("Close"));
        for (auto *a : {m_fill, m_glyph})
            connect(a, &detail::ColorAnimator::updated, this, qOverload<>(&QWidget::update));
        connect(this, &QAbstractButton::pressed, this, [this] { sync(true); });
        connect(this, &QAbstractButton::released, this, [this] { sync(true); });
        connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] { sync(false); });
        sync(false);
    }

    void setRestoreGlyph(bool restore)
    {
        m_restore = restore;
        if (m_kind == Maximize)
            setAccessibleName(restore ? tr("Restore") : tr("Maximize"));
        update();
    }

    void setActive(bool active)
    {
        m_active = active;
        sync(false);
    }

protected:
    void enterEvent(QEnterEvent *e) override
    {
        QAbstractButton::enterEvent(e);
        sync(true);
    }
    void leaveEvent(QEvent *e) override
    {
        QAbstractButton::leaveEvent(e);
        sync(true);
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), m_fill->color());

        const qreal bw = detail::hairline(devicePixelRatioF());
        p.setPen(QPen(m_glyph->color(), bw, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        p.setBrush(Qt::NoBrush);
        const QPointF c(width() / 2.0, height() / 2.0);

        switch (m_kind) {
        case Minimize:
            p.drawLine(QPointF(c.x() - 5, c.y()), QPointF(c.x() + 5, c.y()));
            break;
        case Maximize:
            if (!m_restore) {
                p.drawRoundedRect(QRectF(c.x() - 5, c.y() - 5, 10, 10).adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2),
                                  1.5, 1.5);
            } else {
                // two overlapping squares: back one shows only its top and right edges
                p.drawRoundedRect(QRectF(c.x() - 5, c.y() - 3, 8, 8).adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2),
                                  1.5, 1.5);
                QPainterPath back;
                back.moveTo(c.x() - 2, c.y() - 3 - bw / 2);
                back.lineTo(c.x() - 2, c.y() - 5 + bw / 2);
                back.lineTo(c.x() + 5 - bw / 2, c.y() - 5 + bw / 2);
                back.lineTo(c.x() + 5 - bw / 2, c.y() + 3);
                back.lineTo(c.x() + 3, c.y() + 3);
                p.drawPath(back);
            }
            break;
        case Close:
            p.drawLine(QPointF(c.x() - 5, c.y() - 5), QPointF(c.x() + 5, c.y() + 5));
            p.drawLine(QPointF(c.x() - 5, c.y() + 5), QPointF(c.x() + 5, c.y() - 5));
            break;
        }
    }

private:
    void sync(bool animate)
    {
        const Palette &pal = ThemeManager::instance().palette();
        const bool hover = underMouse(), down = isDown();

        QColor fill = Qt::transparent;
        QColor glyph = m_active ? pal.textPrimary : pal.textDisabled;
        if (m_kind == Close) {
            if (down) { fill = kClosePressed; glyph = Qt::white; }
            else if (hover) { fill = kCloseHover; glyph = Qt::white; }
        } else {
            if (down) fill = pal.subtleFillPressed;
            else if (hover) fill = pal.subtleFillHover;
        }
        m_fill->setTarget(fill, animate);
        m_glyph->setTarget(glyph, animate);
    }

    Kind m_kind;
    detail::ColorAnimator *m_fill;
    detail::ColorAnimator *m_glyph;
    bool m_restore = false;
    bool m_active = true;
};

class FramelessWindow::TitleBar : public QWidget {
public:
    explicit TitleBar(QWidget *parent) : QWidget(parent)
    {
        setFixedHeight(kBarHeight);

        m_icon = new QLabel(this);
        m_icon->setFixedSize(kIconSize, kIconSize);
        m_icon->setScaledContents(true);
        m_icon->hide();
        m_title = new QLabel(this);
        m_title->setAttribute(Qt::WA_TransparentForMouseEvents);

#ifndef Q_OS_MAC
        m_min = new CaptionButton(CaptionButton::Minimize, this);
        m_max = new CaptionButton(CaptionButton::Maximize, this);
        m_close = new CaptionButton(CaptionButton::Close, this);
#endif

        auto *lay = new QHBoxLayout(this);
#ifdef Q_OS_MAC
        // UbiBot patch: the native traffic lights sit at the left edge (see kMacButtonArea).
        lay->setContentsMargins(kMacButtonArea + 4, 0, 12, 0);
#else
        lay->setContentsMargins(12, 0, 0, 0);
#endif
        lay->setSpacing(8);
        lay->addWidget(m_icon);
        lay->addWidget(m_title, 1);
#ifndef Q_OS_MAC
        lay->addSpacing(0);
        lay->addWidget(m_min);
        lay->addWidget(m_max);
        lay->addWidget(m_close);
#endif
        connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] { applyTheme(); });
        applyTheme();
    }

    void setTitle(const QString &t) { m_title->setText(t); }
    void setMenuBar(QMenuBar *bar)
    {
        if (m_menu) {
            layout()->removeWidget(m_menu);
            m_menu->deleteLater();
        }
        m_menu = bar;
        if (bar) {
            bar->setParent(this);
            bar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed); // only as wide as its items: the rest drags
            static_cast<QHBoxLayout *>(layout())->insertWidget(1, bar, 0, Qt::AlignVCenter);
            bar->show();
        }
        m_title->setAlignment(bar ? Qt::AlignCenter : Qt::AlignLeft | Qt::AlignVCenter);
    }
    void setIcon(const QIcon &icon)
    {
        m_icon->setPixmap(icon.pixmap(kIconSize, kIconSize));
        m_icon->setVisible(!icon.isNull());
    }
    void setMaximized(bool max)
    {
#ifndef Q_OS_MAC
        m_max->setRestoreGlyph(max);
#else
        Q_UNUSED(max);
#endif
    }
    void setActive(bool active)
    {
        m_active = active;
        applyTheme();
#ifndef Q_OS_MAC
        for (auto *b : {m_min, m_max, m_close})
            b->setActive(active);
#endif
    }

    CaptionButton *minButton() const { return m_min; }
    CaptionButton *maxButton() const { return m_max; }
    CaptionButton *closeButton() const { return m_close; }
    QLabel *iconLabel() const { return m_icon; }

private:
    void applyTheme()
    {
        const Palette &p = ThemeManager::instance().palette();
        QPalette pal = m_title->palette();
        pal.setColor(QPalette::WindowText, m_active ? p.textPrimary : p.textDisabled);
        m_title->setPalette(pal);
        QFont f = m_title->font();
        f.setPixelSize(12);
        m_title->setFont(f);
    }

    QMenuBar *m_menu = nullptr;
    QLabel *m_icon;
    QLabel *m_title;
    CaptionButton *m_min = nullptr;
    CaptionButton *m_max = nullptr;
    CaptionButton *m_close = nullptr;
    bool m_active = true;
};

FramelessWindow::FramelessWindow(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DontCreateNativeAncestors);

    m_agent = new QWK::WidgetWindowAgent(this);
    m_agent->setup(this);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_titleBar = new TitleBar(this);
    lay->addWidget(m_titleBar);
    m_agent->setTitleBar(m_titleBar);

#ifndef Q_OS_MAC
    m_agent->setSystemButton(QWK::WindowAgentBase::WindowIcon, m_titleBar->iconLabel());
    m_agent->setSystemButton(QWK::WindowAgentBase::Minimize, m_titleBar->minButton());
    m_agent->setSystemButton(QWK::WindowAgentBase::Maximize, m_titleBar->maxButton());
    m_agent->setSystemButton(QWK::WindowAgentBase::Close, m_titleBar->closeButton());

    connect(m_titleBar->minButton(), &QAbstractButton::clicked, this, &QWidget::showMinimized);
    connect(m_titleBar->maxButton(), &QAbstractButton::clicked, this, [this] {
        isMaximized() ? showNormal() : showMaximized();
        // Qt keeps the clicked button "hovered" until the mouse moves after the window
        // geometry changed underneath it; synthesise the leave event.
        QPointer<QWidget> b = m_titleBar->maxButton();
        QTimer::singleShot(0, b, [b] {
            if (b && !b->rect().contains(b->mapFromGlobal(QCursor::pos())))
                QCoreApplication::postEvent(b, new QEvent(QEvent::Leave));
        });
    });
    connect(m_titleBar->closeButton(), &QAbstractButton::clicked, this, &QWidget::close);
#else
    // macOS keeps the native traffic lights (QWindowKit's "no-system-buttons" stays false).
    // UbiBot patch: upstream reserved this area at the right edge, as QWindowKit's demo
    // does; macOS convention is the left edge, which is where the title bar leaves room.
    m_agent->setSystemButtonAreaCallback([](const QSize &size) {
        return QRect(QPoint(0, 0), QSize(kMacButtonArea, size.height()));
    });
#endif

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] {
        applyThemeToNativeFrame();
        update();
    });
    applyThemeToNativeFrame();
    resize(900, 600);
}

FramelessWindow::~FramelessWindow() = default;

void FramelessWindow::setCentralWidget(QWidget *widget)
{
    if (m_central) {
        layout()->removeWidget(m_central);
        m_central->deleteLater();
    }
    m_central = widget;
    if (widget)
        static_cast<QVBoxLayout *>(layout())->addWidget(widget, 1);
}

void FramelessWindow::setMenuBar(QMenuBar *bar)
{
#ifndef Q_OS_MAC
    if (bar == m_menuBar)
        return;
    if (m_menuBar)
        m_agent->setHitTestVisible(m_menuBar, false);
    m_menuBar = bar;
    if (bar) {
        if (auto *mb = qobject_cast<MenuBar *>(bar))
            mb->setCompact(true);
        m_agent->setHitTestVisible(bar, true); // clicks go to the menu bar instead of dragging the window
    }
    m_titleBar->setMenuBar(bar);
#else
    Q_UNUSED(bar);
#endif
}

bool FramelessWindow::setBackdrop(Backdrop backdrop)
{
    static const QString keys[] = {QStringLiteral("mica"), QStringLiteral("mica-alt"),
                                   QStringLiteral("acrylic-material"), QStringLiteral("dwm-blur")};
    for (const QString &k : keys)
        m_agent->setWindowAttribute(k, false);

    QString key;
    switch (backdrop) {
    case Backdrop::None: break;
    case Backdrop::Mica: key = QStringLiteral("mica"); break;
    case Backdrop::MicaAlt: key = QStringLiteral("mica-alt"); break;
    case Backdrop::Acrylic: key = QStringLiteral("acrylic-material"); break;
    }
    const bool ok = key.isEmpty() || m_agent->setWindowAttribute(key, true);
    m_backdrop = ok ? backdrop : Backdrop::None;
    update();
    return ok;
}

void FramelessWindow::applyThemeToNativeFrame()
{
    // keeps the native caption/border/backdrop (Windows) in step with our theme
    m_agent->setWindowAttribute(QStringLiteral("dark-mode"), ThemeManager::instance().isDark());
#ifdef Q_OS_WIN
    // UbiBot patch: force the non-client area to repaint after the dark-mode
    // attribute changes; Windows 10 otherwise keeps the stale 1px frame border
    // until the next activation.
    //
    // NOTE: this is not sufficient on its own. Since Qt 6.8 the Windows platform
    // plugin also writes DWMWA_USE_IMMERSIVE_DARK_MODE on every window from its
    // QStyleHints colour scheme, which tracks the OS rather than the app theme and
    // silently overrides this call (the window stayed dark but kept the light 1px
    // border). The application must keep QStyleHints::setColorScheme() in sync
    // with its theme -- see ui::applyTheme().
    if (testAttribute(Qt::WA_WState_Created)) {
        const HWND hwnd = reinterpret_cast<HWND>(winId());
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        const bool active = GetActiveWindow() == hwnd;
        SendMessageW(hwnd, WM_NCACTIVATE, active ? FALSE : TRUE, 0);
        SendMessageW(hwnd, WM_NCACTIVATE, active ? TRUE : FALSE, 0);
    }
#endif
}

void FramelessWindow::paintEvent(QPaintEvent *)
{
    // with a system backdrop the material shows through, so leave the window transparent
    if (m_backdrop != Backdrop::None)
        return;
    QPainter(this).fillRect(rect(), ThemeManager::instance().palette().windowBg);
}

bool FramelessWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::WindowTitleChange:
        m_titleBar->setTitle(windowTitle());
        break;
    case QEvent::WindowIconChange:
        m_titleBar->setIcon(windowIcon());
        break;
    case QEvent::WindowStateChange:
        m_titleBar->setMaximized(isMaximized());
        break;
    case QEvent::WindowActivate:
        m_titleBar->setActive(true);
        break;
    case QEvent::WindowDeactivate:
        m_titleBar->setActive(false);
        break;
    default:
        break;
    }
    return QWidget::event(e);
}

} // namespace QLight
