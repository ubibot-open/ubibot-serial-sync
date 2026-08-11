#pragma once

#include "core/device_library.h"
#include "core/log_manager.h"
#include "core/serial_manager.h"
#include "core/settings_store.h"
#include "models/command_list_model.h"
#include "models/log_list_model.h"
#include "models/port_list_model.h"

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

    Q_PROPERTY(QString currentModelId READ currentModelId WRITE setCurrentModelId NOTIFY currentModelChanged)
    Q_PROPERTY(QString currentModelDescription READ currentModelDescription NOTIFY currentModelChanged)
    Q_PROPERTY(QStringList modelIds READ modelIds CONSTANT)
    Q_PROPERTY(QString libraryVersion READ libraryVersion CONSTANT)
    Q_PROPERTY(int modelCount READ modelCount CONSTANT)
    Q_PROPERTY(int commandCount READ commandCount CONSTANT)

    Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)

    // Data monitor (right-hand log pane) font, configurable from
    // SettingsAboutDialog.qml and persisted via SettingsStore.
    Q_PROPERTY(QString logFontFamily READ logFontFamily WRITE setLogFontFamily NOTIFY logFontChanged)
    Q_PROPERTY(int logFontSize READ logFontSize WRITE setLogFontSize NOTIFY logFontChanged)

    Q_PROPERTY(QString draftText READ draftText WRITE setDraftText NOTIFY draftTextChanged)
    Q_PROPERTY(bool echoTx READ echoTx WRITE setEchoTx NOTIFY echoTxChanged)
    Q_PROPERTY(bool sendAsHex READ sendAsHex WRITE setSendAsHex NOTIFY sendAsHexChanged)
    Q_PROPERTY(bool repeatSendEnabled READ repeatSendEnabled WRITE setRepeatSendEnabled NOTIFY repeatSendChanged)
    Q_PROPERTY(int repeatIntervalMs READ repeatIntervalMs WRITE setRepeatIntervalMs NOTIFY repeatSendChanged)

    Q_PROPERTY(LogListModel *logModel READ logModel CONSTANT)
    Q_PROPERTY(CommandListModel *commandModel READ commandModel CONSTANT)
    Q_PROPERTY(PortListModel *portListModel READ portListModel CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    bool portOpen() const;
    QString portStatusText() const;
    QString portSummary() const;

    QString currentModelId() const;
    void setCurrentModelId(const QString &id);
    QString currentModelDescription() const;
    // For the connection wizard's model-picker page, which needs to show
    // every model's description without changing the globally-active one.
    Q_INVOKABLE QString modelDescriptionFor(const QString &id) const;
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
    // Installed font families, for the data-monitor font picker in
    // SettingsAboutDialog.qml.
    Q_INVOKABLE QStringList availableFontFamilies() const;

    QString draftText() const { return draftText_; }
    void setDraftText(const QString &text);
    bool echoTx() const { return echoTx_; }
    void setEchoTx(bool on);
    bool sendAsHex() const { return sendAsHex_; }
    void setSendAsHex(bool on);
    bool repeatSendEnabled() const { return repeatEnabled_; }
    void setRepeatSendEnabled(bool on);
    int repeatIntervalMs() const { return repeatIntervalMs_; }
    void setRepeatIntervalMs(int ms);

    LogListModel *logModel() const { return logModel_; }
    CommandListModel *commandModel() const { return commandModel_; }
    PortListModel *portListModel() const { return portListModel_; }

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

    // Parameter-less commands send immediately; ones with parameters do
    // nothing here -- QML reads hasParams from commandModel and opens the
    // params panel itself, then calls sendCommandWithParams().
    Q_INVOKABLE void activateCommandRow(int row);
    Q_INVOKABLE QString commandNameForRow(int row) const;
    Q_INVOKABLE QVariantList paramsForRow(int row) const;
    Q_INVOKABLE QString previewCommand(int row, const QVariantMap &values) const;
    Q_INVOKABLE void sendCommandWithParams(int row, const QVariantMap &values);
    Q_INVOKABLE void toggleFavorite(int row);

    // Returns an error string on failure, empty on success.
    Q_INVOKABLE QString finishWizard(const QString &portName, const QString &modelId);

    Q_INVOKABLE QString suggestedLogBaseName() const;
    Q_INVOKABLE QString suggestedLogDirectory() const;
    // format is "text" | "csv" | "hex". Returns an error string on failure,
    // empty on success.
    Q_INVOKABLE QString saveLog(const QString &directory, const QString &baseFileName, const QString &format,
                                 bool autoRotate);

    Q_INVOKABLE QString checkForLibraryUpdate() const;

signals:
    void connectionChanged();
    void currentModelChanged();
    void currentLanguageChanged();
    void logFontChanged();
    void draftTextChanged();
    void echoTxChanged();
    void sendAsHexChanged();
    void repeatSendChanged();
    void portOpenFailed(const QString &error);
    void wizardFinished();
    void statusMessage(const QString &text);

private:
    QByteArray composeAsciiPayload(const QString &text) const;
    QByteArray composeHexPayload(const QString &text) const;
    void sendLiteral(const QString &text);

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

    LogListModel *logModel_;
    CommandListModel *commandModel_;
    PortListModel *portListModel_;
    QTimer *repeatTimer_;

    QString draftText_;
    bool echoTx_ = true;
    bool sendAsHex_ = false;
    bool repeatEnabled_ = false;
    int repeatIntervalMs_ = 1000;
};
