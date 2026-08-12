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

    // Manual-send history (bottom "Type data to send…" box), newest first,
    // text only -- CommandHistoryModel owns the in-memory timestamps and
    // just persists/reloads the text list here.
    QStringList commandHistory() const;
    void setCommandHistory(const QStringList &entries);

    bool continuousLoggingEnabled() const;
    void setContinuousLoggingEnabled(bool enabled);

    // Data monitor (right-hand log pane) font -- defaults match the
    // hardcoded values DataMonitorView.qml used before this was
    // configurable ("Consolas", 12px).
    QString logFontFamily() const;
    void setLogFontFamily(const QString &family);
    int logFontSize() const;
    void setLogFontSize(int pixelSize);

    // "light" or "dark" -- anything else stored (shouldn't happen outside a
    // hand-edited settings file) falls back to "light". Drives Theme.qml's
    // whole color palette.
    QString themeMode() const;
    void setThemeMode(const QString &mode);

    // Clears the persisted language/font overrides above (the "Settings &
    // About" dialog's own settings) so the next read of each falls back to
    // its built-in default -- system-locale detection for language(),
    // "Consolas"/12 for the font. Leaves everything else (serial config,
    // favorites, window geometry, log preferences) untouched, since those
    // aren't surfaced on that dialog.
    void resetDisplayPreferences();
};
