// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "app/app_controller.h"
#include "core/language_manager.h"
#include "core/settings_store.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#if defined(Q_OS_WIN)
#  include <QAbstractNativeEventFilter>
#  include <QPointer>
#  include <QWindow>
#  include <windows.h>
#endif

#if defined(Q_OS_WIN)
// Corrects the maximized geometry of the frameless main window. Frameless
// windows are WS_POPUP windows underneath, so Windows would maximize them
// to the full monitor; Qt 6.11's own WM_GETMINMAXINFO handling only applies
// the work area *size* while leaving the position anchored at the monitor
// origin (QTBUG-129791). With the taskbar on any edge other than the
// bottom, the window is therefore placed exactly as if the taskbar were at
// the bottom (covering a top/left taskbar, leaving a strip of desktop on
// the opposite side). Anchor the maximized rect to the monitor's actual
// work area instead. Upstream Qt has a fix for this; drop this filter once
// we build against a Qt version that includes it.
class MaximizeToWorkAreaFilter : public QAbstractNativeEventFilter
{
public:
    void setWindow(QWindow *window) { m_window = window; }

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        if (eventType != "windows_generic_MSG" || !m_window)
            return false;
        const auto *msg = static_cast<const MSG *>(message);
        if (msg->message != WM_GETMINMAXINFO
            || msg->hwnd != reinterpret_cast<HWND>(m_window->winId()))
            return false;

        auto *mmi = reinterpret_cast<MINMAXINFO *>(msg->lParam);

        // Consuming the message skips Qt's own min/max tracking size
        // handling (QWindowsGeometryHint::applyToMinMaxInfo); replicate the
        // minimum here so the window's minimum size still applies while
        // interactively resizing.
        const QSize minimum = m_window->minimumSize();
        if (minimum.isValid()) {
            const qreal dpr = m_window->devicePixelRatio();
            mmi->ptMinTrackSize.x = qRound(minimum.width() * dpr);
            mmi->ptMinTrackSize.y = qRound(minimum.height() * dpr);
        }

        const HMONITOR monitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfoW(monitor, &monitorInfo)) {
            mmi->ptMaxPosition.x = monitorInfo.rcWork.left - monitorInfo.rcMonitor.left;
            mmi->ptMaxPosition.y = monitorInfo.rcWork.top - monitorInfo.rcMonitor.top;
            mmi->ptMaxSize.x = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
            mmi->ptMaxSize.y = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
        }
        *result = 0;
        return true;
    }

private:
    QPointer<QWindow> m_window;
};
#endif // Q_OS_WIN

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QGuiApplication::setOrganizationName(QStringLiteral("UbiBot"));
    QGuiApplication::setOrganizationDomain(QStringLiteral("ubibot.com"));
    QGuiApplication::setApplicationName(QStringLiteral("UbiBotSerialAssistant"));
    // APP_VERSION comes from CMakeLists.txt's `project(VERSION ...)` (a
    // single -D compile definition, see there) -- not hardcoded here so
    // there's exactly one place to bump per release rather than several
    // that can drift out of sync.
    QGuiApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/app.png")));

    // Applies the persisted "System font" setting (Settings & About) as the
    // actual process-wide default, read here -- before the QML engine (and
    // its AppController singleton) exist -- so it's the default from the
    // very first frame rather than a later live update. See
    // AppController::buildApplicationFont() for why CJK fallback families
    // get appended rather than substituted outright (Chinese text used to
    // render inconsistently heavy/light across labels since Latin glyphs
    // never triggered the platform's own implicit fallback, but the app's
    // default family never named one explicitly either); the family/size
    // themselves come from SettingsStore, defaulting to the platform's own
    // original font family and one notch above this app's previous ~12px
    // body text.
    SettingsStore settings;
    QGuiApplication::setFont(
        AppController::buildApplicationFont(settings.systemFontFamily(), settings.systemFontSize()));

    // "Fusion" reads as a native desktop control set rather than a mobile
    // one -- closer to the original Widgets version's look than the
    // touch-oriented default styles.
    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    QQmlApplicationEngine engine;

    // qsTr()/QT_TR_NOOP() bindings in QML don't re-evaluate on their own when
    // the active translator changes -- retranslate() is what makes the
    // Settings & About language dropdown update every QML label live
    // instead of only the C++-side (AppController property) strings.
    QObject::connect(&LanguageManager::instance(), &LanguageManager::languageChanged, &engine,
                      [&engine] { engine.retranslate(); });

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.loadFromModule("UbiBot", "Main");

#if defined(Q_OS_WIN)
    if (QWindow *window = qobject_cast<QWindow *>(engine.rootObjects().value(0))) {
        static MaximizeToWorkAreaFilter maximizeFilter;
        maximizeFilter.setWindow(window);
        app.installNativeEventFilter(&maximizeFilter);
    }
#endif

    return app.exec();
}
