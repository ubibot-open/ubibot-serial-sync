#include "app/app_controller.h"
#include "core/language_manager.h"
#include "core/settings_store.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

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

    return app.exec();
}
