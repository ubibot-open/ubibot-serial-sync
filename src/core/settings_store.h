#pragma once

#include "core/language_manager.h"
#include "core/serial_manager.h"

#include <QByteArray>
#include <QSet>
#include <QString>

// Thin, typed façade over QSettings (INI/registry-backed via
// QApplication's organization/application name set in main.cpp). Keeps
// key-string typos in one file instead of scattered across the UI.
class SettingsStore {
public:
    // Returns the stored language code, or -- on first run -- whichever
    // shipped language best matches the system locale (falling back to
    // "en").
    QString language() const;
    void setLanguage(const QString &code);

    SerialConfig lastSerialConfig() const;
    void setLastSerialConfig(const SerialConfig &cfg);

    QString lastModelId() const;
    void setLastModelId(const QString &id);

    // Favorites are keyed "<modelId>/<commandName>" so two models can each
    // have a same-named favorite without colliding.
    bool isFavorite(const QString &modelId, const QString &commandName) const;
    void setFavorite(const QString &modelId, const QString &commandName, bool fav);

    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray &geometry);

    QString lastLogDirectory() const;
    void setLastLogDirectory(const QString &dir);

    bool continuousLoggingEnabled() const;
    void setContinuousLoggingEnabled(bool enabled);
};
