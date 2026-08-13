#include "core/language_manager.h"

#include <QCoreApplication>
#include <QFont>
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
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));

    // Nothing here ever named an explicit default font, so every Text/
    // Label/Control that doesn't set its own font.family falls back to
    // whatever the platform's *implicit* CJK substitution happens to pick
    // whenever it hits a Chinese glyph -- and unlike the Latin UI font,
    // that pick isn't guaranteed to be the same actual font file (or to
    // even have a real bold weight rather than a synthesized/emboldened
    // one) at every pixel size used across the app. That's what made
    // Chinese text render inconsistently heavy in some labels and light in
    // others while English never showed the problem: Latin glyphs are
    // covered by the UI font itself and never trigger that fallback path.
    // Querying the platform's own default first and only *appending* CJK
    // candidates (rather than replacing it outright) keeps every other
    // script's rendering exactly as it already was on every platform --
    // this only changes what happens once Qt reaches a glyph the platform
    // default can't cover itself.
    QFont defaultFont = QGuiApplication::font();
    QStringList fontFamilies = defaultFont.families();
    if (fontFamilies.isEmpty()) fontFamilies << defaultFont.family();
    fontFamilies << QStringLiteral("Microsoft YaHei UI") << QStringLiteral("Microsoft YaHei")
                 << QStringLiteral("PingFang SC") << QStringLiteral("Noto Sans CJK SC");
    defaultFont.setFamilies(fontFamilies);
    QGuiApplication::setFont(defaultFont);

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
