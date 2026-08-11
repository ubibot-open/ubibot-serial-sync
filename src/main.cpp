#include "core/language_manager.h"

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
    QGuiApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));

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
