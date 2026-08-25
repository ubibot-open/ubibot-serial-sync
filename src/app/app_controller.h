#pragma once

#include "core/batch_command.h"
#include "core/device_library.h"
#include "core/device_library_update_client.h"
#include "core/log_manager.h"
#include "core/serial_manager.h"
#include "core/settings_store.h"
#include "models/batch_command_model.h"
#include "models/command_history_model.h"
#include "models/command_list_model.h"
#include "models/log_list_model.h"
#include "models/port_list_model.h"

#include <QFont>
#include <QObject>
#include <QQmlEngine>
#include <QVariantList>
#include <QVariantMap>

// Q_PROPERTY pointer types must be complete (not just forward-declared) --
// Qt6's stricter QMetaType requires a fully-defined type behind LogModel*/
// etc., so these are #included above rather than forward-declared.
class QTimer;

// The single C++ facade the QML UI talks to. Owns every backend piece
// (serial I/O, the log, the device command library, persisted settings) and
// exposes them as QML-friendly properties/invokables/models. All actual
// logic -- opening a port, resolving an AT command template, exporting a
// log file, deciding on a default log directory -- lives here in C++; the
// QML files only render state and forward user actions to it.
class AppController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool portOpen READ portOpen NOTIFY connectionChanged)
    Q_PROPERTY(QString portStatusText READ portStatusText NOTIFY connectionChanged)
    Q_PROPERTY(QString portSummary READ portSummary NOTIFY connectionChanged)

    // The app's own release version (see CMakeLists.txt's top-of-file
    // comment for where this actually comes from/how to bump it) -- shown
    // in the custom title bar and Settings & About. qtVersion is the Qt
    // build it was compiled against, shown alongside it in Settings &
    // About; neither needs a NOTIFY, both are fixed for the process's
    // whole lifetime. buildTime is the CMake-configure-time timestamp from
    // the same compile-definition mechanism (see CMakeLists.txt), shown in
    // the About dialog.
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    Q_PROPERTY(QString buildTime READ buildTime CONSTANT)

    Q_PROPERTY(QString currentModelId READ currentModelId WRITE setCurrentModelId NOTIFY currentModelChanged)
    Q_PROPERTY(QString currentModelDescription READ currentModelDescription NOTIFY currentModelChanged)
    // These four used to be CONSTANT (the library only ever came from the
    // bundled resources/devices.json, loaded once at startup). Now that a
    // successful "download and apply" (see downloadLibraryUpdate() below)
    // can replace the library's contents in place while the app is running,
    // they need libraryChanged() so QML rebinds instead of showing stale
    // model/command counts until a restart.
    Q_PROPERTY(QStringList modelIds READ modelIds NOTIFY libraryChanged)
    Q_PROPERTY(QString libraryVersion READ libraryVersion NOTIFY libraryChanged)
    Q_PROPERTY(int modelCount READ modelCount NOTIFY libraryChanged)
    Q_PROPERTY(int commandCount READ commandCount NOTIFY libraryChanged)

    // Device-library remote update state (see core/device_library_update_client.h
    // and docs/device-library-update-protocol.md) -- all driven by
    // checkForLibraryUpdate()/downloadLibraryUpdate() below and read by the
    // "Command library" section of SettingsAboutDialog.qml.
    // "idle" | "checking" | "upToDate" | "updateAvailable" | "downloading" | "error"
    Q_PROPERTY(QString libraryUpdateState READ libraryUpdateState NOTIFY libraryUpdateStateChanged)
    // Human-readable status/error/changelog text for whatever
    // libraryUpdateState currently is -- what the dialog actually displays.
    Q_PROPERTY(QString libraryUpdateMessage READ libraryUpdateMessage NOTIFY libraryUpdateStateChanged)
    // Version string reported by the last successful /version check; empty
    // until one has succeeded.
    Q_PROPERTY(QString remoteLibraryVersion READ remoteLibraryVersion NOTIFY libraryUpdateStateChanged)
    // True only when a check found a newer version AND the app itself
    // satisfies that version's minAppVersion -- gates the "Download and
    // apply" button in SettingsAboutDialog.qml.
    Q_PROPERTY(bool libraryUpdateAvailable READ libraryUpdateAvailable NOTIFY libraryUpdateStateChanged)

    Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)

    // Data monitor (right-hand log pane) font, configurable from
    // SettingsAboutDialog.qml and persisted via SettingsStore.
    Q_PROPERTY(QString logFontFamily READ logFontFamily WRITE setLogFontFamily NOTIFY logFontChanged)
    Q_PROPERTY(int logFontSize READ logFontSize WRITE setLogFontSize NOTIFY logFontChanged)

    // App-wide UI font -- everything *except* the data monitor pane above.
    // Configurable from SettingsAboutDialog.qml's "System font" section;
    // Theme.qml exposes it as baseFontFamily/baseFontSize for QML to bind
    // against, and the setters below also re-apply it as the actual process
    // default via QGuiApplication::setFont() (see buildApplicationFont()).
    Q_PROPERTY(QString systemFontFamily READ systemFontFamily WRITE setSystemFontFamily NOTIFY systemFontChanged)
    Q_PROPERTY(int systemFontSize READ systemFontSize WRITE setSystemFontSize NOTIFY systemFontChanged)

    // "light" or "dark" -- Theme.qml (the app-wide color palette singleton)
    // reads this to decide which set of colors every Theme.* property
    // resolves to.
    Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)

    // Which serial port is currently picked, shared between the port combo
    // boxes on the "Device commands" and "Serial" panels (only one of which
    // is visible at a time, but both should show/drive the same choice) --
    // whichever one the user last touched wins, and "Open port" in the
    // toolbar opens whatever this holds.
    Q_PROPERTY(QString selectedPortName READ selectedPortName WRITE setSelectedPortName NOTIFY selectedPortNameChanged)

    Q_PROPERTY(QString draftText READ draftText WRITE setDraftText NOTIFY draftTextChanged)
    Q_PROPERTY(bool echoTx READ echoTx WRITE setEchoTx NOTIFY echoTxChanged)
    Q_PROPERTY(bool sendAsHex READ sendAsHex WRITE setSendAsHex NOTIFY sendAsHexChanged)
    // When on, every outgoing send -- the manual box (ASCII or HEX) and
    // one-click device-library commands alike -- gets a CRC16/MODBUS
    // checksum appended to its payload before it goes out. See
    // composeAsciiPayload()/composeHexPayload() for where that actually
    // happens and core/crc16.h for the algorithm.
    Q_PROPERTY(bool crcEnabled READ crcEnabled WRITE setCrcEnabled NOTIFY crcEnabledChanged)
    Q_PROPERTY(bool repeatSendEnabled READ repeatSendEnabled WRITE setRepeatSendEnabled NOTIFY repeatSendChanged)
    Q_PROPERTY(int repeatIntervalMs READ repeatIntervalMs WRITE setRepeatIntervalMs NOTIFY repeatSendChanged)

    // "Batch commands" (CommandLibraryPanel.qml's own batch dialog) --
    // batchRunning/runningBatchRow/batchProgressText describe whatever
    // batch is currently mid-send (runningBatchRow is the row index into
    // batchCommandModel, -1 when nothing is running) so the dialog can
    // highlight the right row and show live "N / total" progress
    // regardless of whether the dialog was the thing that started it.
    Q_PROPERTY(BatchCommandModel *batchCommandModel READ batchCommandModel CONSTANT)
    Q_PROPERTY(bool batchRunning READ batchRunning NOTIFY batchStateChanged)
    Q_PROPERTY(int runningBatchRow READ runningBatchRow NOTIFY batchStateChanged)
    Q_PROPERTY(QString batchProgressText READ batchProgressText NOTIFY batchStateChanged)

    Q_PROPERTY(LogListModel *logModel READ logModel CONSTANT)
    Q_PROPERTY(CommandListModel *commandModel READ commandModel CONSTANT)
    Q_PROPERTY(PortListModel *portListModel READ portListModel CONSTANT)
    Q_PROPERTY(CommandHistoryModel *commandHistoryModel READ commandHistoryModel CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    QString appVersion() const;
    QString qtVersion() const;
    QString buildTime() const;

    bool portOpen() const;
    QString portStatusText() const;
    QString portSummary() const;

    QString currentModelId() const;
    void setCurrentModelId(const QString &id);
    QString currentModelDescription() const;
    QStringList modelIds() const;
    QString libraryVersion() const;
    int modelCount() const;
    int commandCount() const;

    QString currentLanguage() const;
    void setCurrentLanguage(const QString &code);

    QString logFontFamily() const;
    void setLogFontFamily(const QString &family);
    int logFontSize() const;
    void setLogFontSize(int pixelSize);
    QString systemFontFamily() const;
    void setSystemFontFamily(const QString &family);
    int systemFontSize() const;
    void setSystemFontSize(int pixelSize);
    // Builds the process-wide default QFont for (family, pixelSize) --
    // `family` empty keeps whatever QGuiApplication::font() already has.
    // Either way, CJK fallback families get appended (never substituted
    // outright) so Chinese glyphs render consistently regardless of
    // platform/user pick; see main.cpp's original comment for why. Shared
    // between main.cpp's startup call and setSystemFontFamily/
    // setSystemFontSize above so the font is always built the same way
    // whether it's applied before or after the QML engine starts.
    static QFont buildApplicationFont(const QString &family, int pixelSize);
    QString themeMode() const;
    void setThemeMode(const QString &mode);
    // Installed font families, for the data-monitor/system font pickers in
    // SettingsAboutDialog.qml.
    Q_INVOKABLE QStringList availableFontFamilies() const;
    // "Restore defaults" on SettingsAboutDialog.qml -- resets every setting
    // exposed on that dialog (language, data monitor font) back to its
    // built-in default.
    Q_INVOKABLE void restoreDefaultSettings();

    QString selectedPortName() const { return selectedPortName_; }
    void setSelectedPortName(const QString &name);

    QString draftText() const { return draftText_; }
    void setDraftText(const QString &text);
    bool echoTx() const { return echoTx_; }
    void setEchoTx(bool on);
    bool sendAsHex() const { return sendAsHex_; }
    void setSendAsHex(bool on);
    bool crcEnabled() const { return crcEnabled_; }
    void setCrcEnabled(bool on);
    bool repeatSendEnabled() const { return repeatEnabled_; }
    void setRepeatSendEnabled(bool on);
    int repeatIntervalMs() const { return repeatIntervalMs_; }
    void setRepeatIntervalMs(int ms);

    BatchCommandModel *batchCommandModel() const { return batchCommandModel_; }
    bool batchRunning() const { return batchRunningRow_ >= 0; }
    int runningBatchRow() const { return batchRunningRow_; }
    QString batchProgressText() const;

    LogListModel *logModel() const { return logModel_; }
    CommandListModel *commandModel() const { return commandModel_; }
    PortListModel *portListModel() const { return portListModel_; }
    CommandHistoryModel *commandHistoryModel() const { return commandHistoryModel_; }

    // {code, nativeName} for every shipped language -- backs the dropdown in
    // SettingsAboutDialog.qml.
    Q_INVOKABLE QVariantList availableLanguages() const;

    // dataBits/parity/stopBits/flowControl are the int values SerialOptions'
    // corresponding option lists hand back (i.e. the underlying
    // QSerialPort::* enum values).
    Q_INVOKABLE bool openPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits,
                               int flowControl);
    Q_INVOKABLE void closePort();

    // Sends draftText() (interpreted per sendAsHex()); also what the
    // repeat-send timer calls on every tick.
    Q_INVOKABLE void sendManualText();
    Q_INVOKABLE void clearDraft();

    // The device command library is a quick way to *find* a command, not to
    // fire it straight at the port -- it doesn't know or care whether a port
    // is even open. Every row click stages the command's literal text into
    // the manual-send box (draftText) for the user to review/edit and send
    // themselves from there; nothing here ever writes to the serial port.
    // Params-bearing commands go through the params panel first (QML reads
    // hasParams from commandModel and opens it, which then calls
    // loadCommandWithParamsIntoDraft() below once the user fills in
    // values); everything else calls this directly.
    Q_INVOKABLE void loadCommandIntoDraft(int row);
    Q_INVOKABLE QString commandNameForRow(int row) const;
    Q_INVOKABLE QVariantList paramsForRow(int row) const;
    Q_INVOKABLE QString previewCommand(int row, const QVariantMap &values) const;
    Q_INVOKABLE void loadCommandWithParamsIntoDraft(int row, const QVariantMap &values);

    // "My templates" -- user-authored quick-send text, unrelated to any
    // device model, edited/deleted from CommandLibraryPanel.qml's own
    // "+ New template" button/row context menu rather than shipped in
    // devices.json. See CommandListModel::addCustomTemplate() and friends
    // for the actual storage; these three just forward to commandModel_.
    Q_INVOKABLE void addCustomTemplate(const QString &name, const QString &content);
    Q_INVOKABLE void updateCustomTemplate(int row, const QString &name, const QString &content);
    Q_INVOKABLE void removeCustomTemplate(int row);

    // Drag-to-reorder the command list (CommandLibraryPanel.qml's delegate)
    // -- forwards to CommandListModel::moveRow(), which persists the result.
    Q_INVOKABLE void moveCommandRow(int from, int to);

    // "Batch commands" -- a user-authored sequence of {text, isHex, crc}
    // steps sent one at a time at a fixed interval, picked from
    // CommandLibraryPanel.qml's own batch dialog. add/update/remove/
    // stepsForRow forward straight to batchCommandModel_ (same reasoning as
    // addCustomTemplate and friends above); start/stop own the actual send
    // engine (batchTimer_) since that needs serial_/logManager_ access the
    // model doesn't have. `steps` is a QVariantList of {text, isHex, crc}
    // maps.
    Q_INVOKABLE void addBatchCommand(const QString &name, int intervalMs, const QVariantList &steps);
    Q_INVOKABLE void updateBatchCommand(int row, const QString &name, int intervalMs, const QVariantList &steps);
    Q_INVOKABLE void removeBatchCommand(int row);
    Q_INVOKABLE QVariantList stepsForBatchRow(int row) const;
    // No-op if the port isn't open, the row is out of range, or it has no
    // steps. Stops whatever batch is already running first (if any), so
    // there's only ever one in flight.
    Q_INVOKABLE void startBatchCommand(int row);
    // Safe to call whether or not a batch is actually running.
    Q_INVOKABLE void stopBatchCommand();

    // {present, baudRate, dataBits, parity, stopBits, flowControl} for the
    // given model id -- the last four are the raw QSerialPort enum ints, so
    // QML can feed them straight to a SerialOptions combo's indexOfValue().
    // present is false (and the rest absent) when the model has no "serial"
    // block in devices.json. Backs SerialSettingsPanel.qml's "selecting a
    // model overwrites the serial fields" behavior.
    Q_INVOKABLE QVariantMap serialDefaultsForModel(const QString &modelId) const;

    Q_INVOKABLE QString suggestedLogBaseName() const;
    Q_INVOKABLE QString suggestedLogDirectory() const;
    // format is "text" | "csv" | "hex". Returns an error string on failure,
    // empty on success.
    Q_INVOKABLE QString saveLog(const QString &directory, const QString &baseFileName, const QString &format,
                                 bool autoRotate);

    QString libraryUpdateState() const { return libraryUpdateState_; }
    QString libraryUpdateMessage() const { return libraryUpdateMessage_; }
    QString remoteLibraryVersion() const { return remoteLibraryVersion_; }
    bool libraryUpdateAvailable() const { return libraryUpdateAvailable_; }

    // Kicks off an async GET {baseUrl}/version (see
    // core/device_library_update_client.h); results land on the
    // libraryUpdateState/libraryUpdateMessage/remoteLibraryVersion/
    // libraryUpdateAvailable properties above via libraryUpdateStateChanged
    // once the request completes -- this method itself returns immediately.
    // A no-op-ish immediate "not configured" result when .env doesn't set
    // DEVICE_LIBRARY_API_BASE_URL.
    Q_INVOKABLE void checkForLibraryUpdate();
    // Only meaningful (and only exposed as actionable in the UI) once
    // libraryUpdateAvailable is true. Downloads {baseUrl}/latest, and on
    // success replaces the in-memory library, persists it via
    // SettingsStore::setCachedLibraryJson(), and refreshes commandModel_ --
    // all visible immediately, no restart required.
    Q_INVOKABLE void downloadLibraryUpdate();

signals:
    void connectionChanged();
    void currentModelChanged();
    void selectedPortNameChanged();
    void currentLanguageChanged();
    void logFontChanged();
    void systemFontChanged();
    void themeModeChanged();
    void draftTextChanged();
    void echoTxChanged();
    void sendAsHexChanged();
    void crcEnabledChanged();
    void repeatSendChanged();
    void batchStateChanged();
    void portOpenFailed(const QString &error);
    void statusMessage(const QString &text);
    // Fired whenever the device library's contents are wholesale replaced
    // (currently only downloadLibraryUpdate() succeeding) -- modelIds/
    // libraryVersion/modelCount/commandCount all re-read from library_ then.
    void libraryChanged();
    void libraryUpdateStateChanged();

private:
    // `crc` is explicit (rather than always reading crcEnabled_) so a batch
    // step's own per-step CRC option (independent of the global toggle) can
    // reuse these same three helpers -- see sendNextBatchStep().
    QByteArray composeAsciiPayload(const QString &text, bool crc) const;
    QByteArray composeHexPayload(const QString &text, bool crc) const;
    // " [CRC XX XX]" (the exact bytes composeAsciiPayload()/composeHexPayload()
    // append for this same data) when `crc` is on, else empty -- appended to
    // the TX echo log line so an ASCII-mode send's log entry still shows
    // what actually went out, since (unlike the HEX-mode echo, which already
    // logs the final CRC-included payload as hex) the ASCII echo logs the
    // human-typed text, not raw wire bytes.
    QString crcEchoSuffix(const QByteArray &data, bool crc) const;

    // Sends batchQueue_.steps[batchStepIndex_] and advances batchStepIndex_;
    // called once immediately from startBatchCommand() (so a run's first
    // step goes out right away instead of only after the first interval
    // elapses) and then once per batchTimer_ tick. Stops the run (via
    // stopBatchCommand()) once the queue is exhausted, or if the port
    // somehow closed mid-run.
    void sendNextBatchStep();

    // Raw serial reads land in arbitrary, driver-chosen chunk sizes -- a
    // single logical line from the device routinely arrives as several
    // readyRead signals (each becoming its own log entry if appended
    // as-is), which is what fragmented one line of device output across
    // several rows, each with its own timestamp, in the data monitor.
    // Buffering here and only appending on a line boundary is what a real
    // terminal does. Two escape hatches keep this from ever *hiding* data
    // a plain per-chunk append wouldn't have: rxFlushTimer_ flushes
    // whatever's buffered after a short quiet spell (so a bare prompt with
    // no trailing '\n', e.g. an interactive shell waiting on input, still
    // shows up promptly instead of sitting invisible until more bytes
    // arrive) and handleIncomingData() flushes outright once the buffer
    // grows past kMaxBufferedLine (so a binary/non-line-oriented stream
    // with no '\n' at all can't buffer forever). flushRxLineBuffer() also
    // runs on port close, so a stray final partial line isn't dropped.
    void handleIncomingData(const QByteArray &data);
    void flushRxLineBuffer();
    QByteArray rxLineBuffer_;
    QTimer *rxFlushTimer_;

    DeviceLibrary library_;
    SettingsStore settings_;
    SerialManager serial_;
    LogManager logManager_;
    DeviceLibraryUpdateClient *libraryUpdateClient_;

    QString libraryUpdateState_ = QStringLiteral("idle");
    QString libraryUpdateMessage_;
    QString remoteLibraryVersion_;
    bool libraryUpdateAvailable_ = false;

    LogListModel *logModel_;
    CommandListModel *commandModel_;
    PortListModel *portListModel_;
    CommandHistoryModel *commandHistoryModel_;
    BatchCommandModel *batchCommandModel_;
    QTimer *repeatTimer_;
    QTimer *batchTimer_;
    // Snapshot of the batch being sent, taken when startBatchCommand() opens
    // it -- so editing or deleting the saved entry in batchCommandModel_
    // mid-run can't corrupt an in-flight send. batchRunningRow_ is the row
    // index into batchCommandModel_ this snapshot came from (-1 when no
    // batch is running); batchStepIndex_ is how many of its steps have
    // been sent so far.
    BatchCommand batchQueue_;
    int batchRunningRow_ = -1;
    int batchStepIndex_ = 0;

    QString selectedPortName_;
    QString draftText_;
    bool echoTx_ = true;
    bool sendAsHex_ = false;
    bool crcEnabled_ = false;
    bool repeatEnabled_ = false;
    int repeatIntervalMs_ = 1000;
};
