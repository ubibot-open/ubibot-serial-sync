#include "core/env_config.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

const EnvConfig &EnvConfig::instance() {
    static const EnvConfig cfg;
    return cfg;
}

EnvConfig::EnvConfig() {
    // Production/deploy layout: ".env" sits next to the .exe, copied there by
    // the release/packaging step (not by this build) -- see
    // docs/device-library-update-protocol.md#2.
    loadFile(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral(".env")));
    // Dev convenience: running straight out of the build tree with a ".env"
    // dropped into the current working directory. Loaded second so it wins
    // over the deploy-layout file above when both happen to exist.
    loadFile(QDir::current().filePath(QStringLiteral(".env")));
}

void EnvConfig::loadFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) continue;

        const int eq = line.indexOf(QLatin1Char('='));
        if (eq <= 0) continue;  // no "=", or line starts with one -- not a KEY=VALUE line

        const QString key = line.left(eq).trimmed();
        QString value = line.mid(eq + 1).trimmed();
        // Strip one layer of matching quotes, e.g. KEY="value with spaces".
        if (value.size() >= 2 &&
            ((value.front() == QLatin1Char('"') && value.back() == QLatin1Char('"')) ||
             (value.front() == QLatin1Char('\'') && value.back() == QLatin1Char('\'')))) {
            value = value.mid(1, value.size() - 2);
        }
        if (!key.isEmpty()) values_.insert(key, value);
    }
}

QString EnvConfig::value(const QString &key, const QString &fallback) const {
    return values_.value(key, fallback);
}
