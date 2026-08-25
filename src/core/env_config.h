#pragma once

#include <QHash>
#include <QString>

// Reads key=value pairs out of a ".env" file that is deliberately NOT part of
// this repo (see .gitignore / .env.example) -- the device-library update
// server's URL (and optional API key) can differ per environment and
// shouldn't require a rebuild to change. See
// docs/device-library-update-protocol.md for the full picture.
//
// Looked up next to the running executable first (the production/deploy
// layout), then the current working directory (dev convenience when running
// straight out of the build tree); a key found in the second location
// overrides the first. Neither file existing, or a key simply being absent,
// both just mean "not configured" -- value() returns `fallback` (empty by
// default) and callers (DeviceLibraryUpdateClient) treat an empty base URL as
// "remote update disabled, bundled devices.json only" rather than an error.
class EnvConfig {
public:
    static const EnvConfig &instance();

    QString value(const QString &key, const QString &fallback = QString()) const;

private:
    EnvConfig();
    void loadFile(const QString &path);

    QHash<QString, QString> values_;
};
