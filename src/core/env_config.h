#pragma once

#include <QHash>
#include <QString>

// Reads key=value pairs out of a ".env" file that is deliberately NOT part of
// this repo (see .gitignore / .env.example) -- the device-library update
// server's URL (and optional API key) can differ per environment and
// shouldn't require a rebuild to change. See
// docs/device-library-update-protocol.md for the full picture.
//
// Looked up in, in order: next to the running executable (the production/
// deploy layout), the current working directory (dev convenience when
// running straight out of the build tree), then this checkout's own source
// root (APP_SOURCE_DIR, a compile-time define from CMakeLists.txt) -- the
// last one exists so a ".env" left at the repo root is still found when
// running from Qt Creator or a plain build/ tree, whose default working
// directory is the build dir, not the source dir. Each location loaded
// overrides keys already loaded from an earlier one. Neither file existing
// anywhere, or a key simply being absent, both just mean "not configured" --
// value() returns `fallback` (empty by default) and callers
// (DeviceLibraryUpdateClient) treat an empty base URL as "remote update
// disabled, bundled devices.json only" rather than an error.
class EnvConfig {
public:
    static const EnvConfig &instance();

    QString value(const QString &key, const QString &fallback = QString()) const;

private:
    EnvConfig();
    void loadFile(const QString &path);

    QHash<QString, QString> values_;
};
