#pragma once

#include <QString>
#include <QVector>

// One step of a user-authored "batch command" (CommandLibraryPanel.qml's
// batch dialog) -- a literal command text plus its own ASCII/HEX
// interpretation and CRC option. Deliberately per-step rather than reusing
// AppController's global sendAsHex/crcEnabled toggles: a single batch
// commonly needs to mix formats (e.g. a HEX config frame followed by a
// plain ASCII "AT+SAVE\r\n"), which a single pair of app-wide toggles can't
// express.
struct BatchCommandStep {
    QString text;
    bool isHex = false;
    bool crcEnabled = false;
};

// A user-authored sequence of steps, sent one at a time at a fixed interval
// once picked from CommandLibraryPanel.qml's "Batch commands" dialog. `id`
// is a QUuid-derived stable key, same reasoning as CustomCommandTemplate's
// own id (see settings_store.h). Persisted via SettingsStore::
// batchCommands(); AppController snapshots one of these into its own
// batchQueue_ when a run starts, so editing/deleting the saved entry
// mid-run can't corrupt an in-flight send.
struct BatchCommand {
    QString id;
    QString name;
    int intervalMs = 500;
    QVector<BatchCommandStep> steps;
};
