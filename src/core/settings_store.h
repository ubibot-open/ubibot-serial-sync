#pragma once

#include "core/language_manager.h"
#include "core/serial_manager.h"

#include <QByteArray>
#include <QSet>
#include <QString>
#include <QVector>

// A user-authored quick-send template from the "Device commands" panel's
// "My templates" filter -- a plain {name, content} pair with no
// protocol/params concept and no per-device-model association (see
// CommandListModel, which merges these into the same row list as the
// bundled devices.json commands regardless of which model is selected).
// `id` is a QUuid-derived stable key so renaming a template doesn't lose
// track of which stored entry it corresponds to.
struct CustomCommandTemplate {
    QString id;
    QString name;
    QString content;
};

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

    // The user's own drag-to-reorder of the "Device commands" panel's
    // merged command list (bundled devices.json commands + "My templates"),
    // as a flat list of commandKeys (DeviceCommand::id if present, else
    // name.zh -- same key favorites used to use) in display order. Empty
    // until the user actually drags a row, at which point
    // CommandListModel::moveRow() captures the *complete* current order
    // here (not just the two rows that swapped), so every row -- reordered
    // or not -- has an explicit position to fall back on. See
    // CommandListModel::rebuild() for how a row with no entry here (a
    // different device model's own commands, say, never touched by a drag)
    // gets positioned once this is non-empty.
    QStringList commandOrder() const;
    void setCommandOrder(const QStringList &order);

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

    // The full "My templates" list, in display order. Small, user-curated
    // (a handful to a few dozen entries at most) -- stored as one JSON
    // array under a single QSettings key rather than its own file, same
    // reasoning as commandHistory() above. CommandListModel is the only
    // reader/writer (via AppController's addCustomTemplate/
    // updateCustomTemplate/removeCustomTemplate), always read-modify-write
    // of the whole list.
    QVector<CustomCommandTemplate> customTemplates() const;
    void setCustomTemplates(const QVector<CustomCommandTemplate> &templates);

    // Data monitor (right-hand log pane) font -- defaults match the
    // hardcoded values DataMonitorView.qml used before this was
    // configurable ("Consolas", 12px).
    QString logFontFamily() const;
    void setLogFontFamily(const QString &family);
    int logFontSize() const;
    void setLogFontSize(int pixelSize);

    // App-wide UI font -- everything *except* the data monitor pane above,
    // which keeps its own logFontFamily/logFontSize. Configurable from
    // SettingsAboutDialog.qml's "System font" section. logFontFamily's
    // default is a fixed literal ("Consolas"); this one instead defaults to
    // the platform's own default family (captured once -- see the .cpp) so
    // a fresh install matches whatever the OS already looked like. The
    // default size is one notch above the ~12px body text this app's own
    // QML used everywhere before this setting existed, per user feedback
    // that the stock UI read too small.
    QString systemFontFamily() const;
    void setSystemFontFamily(const QString &family);
    int systemFontSize() const;
    void setSystemFontSize(int pixelSize);

    // "light" or "dark" -- anything else stored (shouldn't happen outside a
    // hand-edited settings file), including nothing stored at all yet (a
    // fresh install), falls back to "dark". Drives Theme.qml's whole color
    // palette.
    QString themeMode() const;
    void setThemeMode(const QString &mode);

    // Clears the persisted language/font overrides above (the "Settings &
    // About" dialog's own settings) so the next read of each falls back to
    // its built-in default -- system-locale detection for language(),
    // "Consolas"/12 for the data monitor font, the platform default family
    // and 13px for the system font. Leaves everything else (serial config,
    // favorites, window geometry, log preferences) untouched, since those
    // aren't surfaced on that dialog.
    void resetDisplayPreferences();
};
