#include "ui/mainwindow.h"
#include "ui/styles.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setOrganizationName(QStringLiteral("UbiBot"));
    QApplication::setOrganizationDomain(QStringLiteral("ubibot.com"));
    QApplication::setApplicationName(QStringLiteral("UbiBotSerialAssistant"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));

    app.setStyleSheet(Styles::appStyleSheet());

    MainWindow window;
    window.show();

    return app.exec();
}
