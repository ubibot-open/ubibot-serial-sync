#include "app/app_controller.h"

#include "core/language_manager.h"
#include "models/command_history_model.h"
#include "models/command_list_model.h"
#include "models/log_list_model.h"
#include "models/port_list_model.h"

#include <QDateTime>
#include <QDir>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QTimer>

namespace {
QHash<QString, QString> toHash(const QVariantMap &values) {
    QHash<QString, QString> h;
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) h.insert(it.key(), it.value().toString());
    return h;
}
}  // namespace

AppController::AppController(QObject *parent) : QObject(parent) {
    library_.loadFromResource();
    LanguageManager::instance().setLanguage(settings_.language());

    logModel_ = new LogListModel(&logManager_, this);
    commandModel_ = new CommandListModel(&library_, &settings_, this);
    portListModel_ = new PortListModel(this);
    commandHistoryModel_ = new CommandHistoryModel(&settings_, this);

    repeatTimer_ = new QTimer(this);
    connect(repeatTimer_, &QTimer::timeout, this, &AppController::sendManualText);

    rxFlushTimer_ = new QTimer(this);
    rxFlushTimer_->setSingleShot(true);
    connect(rxFlushTimer_, &QTimer::timeout, this, &AppController::flushRxLineBuffer);

    connect(&serial_, &SerialManager::dataReceived, this, &AppController::handleIncomingData);
    connect(&serial_, &SerialManager::errorOccurred, this,
            [this](const QString &msg) { logManager_.append(LogKind::Err, msg.toUtf8()); });
    connect(&serial_, &SerialManager::opened, this, [this](const SerialConfig &) { emit connectionChanged(); });
    connect(&serial_, &SerialManager::closed, this, [this] {
        repeatTimer_->stop();
        flushRxLineBuffer();
        emit connectionChanged();
    });

    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, [this](const QString &code) {
        settings_.setLanguage(code);
        emit currentLanguageChanged();
        emit currentModelChanged();  // the model description is localized text
    });

    const QString lastModel = settings_.lastModelId();
    commandModel_->setModelId(lastModel.isEmpty() ? library_.modelIds().value(0) : lastModel);
}

AppController::~AppController() = default;

bool AppController::portOpen() const { return serial_.isOpen(); }

QString AppController::portStatusText() const {
    const QString port = serial_.currentConfig().portName;
    const QString label = port.isEmpty() ? QStringLiteral("—") : port;
    return serial_.isOpen() ? tr("%1 OPEN").arg(label) : tr("%1 CLOSED").arg(label);
}

QString AppController::portSummary() const {
    return serial_.isOpen() ? serial_.portSummary() : QStringLiteral("—");
}

QString AppController::currentModelId() const { return commandModel_->modelId(); }

void AppController::setCurrentModelId(const QString &id) {
    if (commandModel_->modelId() == id) return;
    commandModel_->setModelId(id);
    settings_.setLastModelId(id);
    emit currentModelChanged();
}

QString AppController::currentModelDescription() const {
    const DeviceModel *m = library_.model(commandModel_->modelId());
    return m ? m->description.text() : QString();
}

QString AppController::modelDescriptionFor(const QString &id) const {
    const DeviceModel *m = library_.model(id);
    return m ? m->description.text() : QString();
}

QStringList AppController::modelIds() const { return library_.modelIds(); }
QString AppController::libraryVersion() const { return library_.version(); }
int AppController::modelCount() const { return int(library_.models().size()); }
int AppController::commandCount() const { return library_.commandCount(); }

QString AppController::currentLanguage() const { return LanguageManager::instance().language(); }

void AppController::setCurrentLanguage(const QString &code) { LanguageManager::instance().setLanguage(code); }

QString AppController::logFontFamily() const { return settings_.logFontFamily(); }

void AppController::setLogFontFamily(const QString &family) {
    if (settings_.logFontFamily() == family) return;
    settings_.setLogFontFamily(family);
    emit logFontChanged();
}

int AppController::logFontSize() const { return settings_.logFontSize(); }

void AppController::setLogFontSize(int pixelSize) {
    if (settings_.logFontSize() == pixelSize) return;
    settings_.setLogFontSize(pixelSize);
    emit logFontChanged();
}

QStringList AppController::availableFontFamilies() const { return QFontDatabase::families(); }

void AppController::restoreDefaultSettings() {
    settings_.resetDisplayPreferences();

    // language() now re-derives the system-locale default since
    // resetDisplayPreferences() cleared the stored override. Re-applying it
    // through LanguageManager (rather than just settings_.setLanguage())
    // keeps translations in sync even when the default happens to equal
    // whatever was already active. currentLanguageChanged/currentModelChanged
    // are emitted unconditionally for the same reason -- LanguageManager's
    // own signal only fires on an actual change, but the dialog's combo box
    // still needs telling to re-sync its displayed index.
    LanguageManager::instance().setLanguage(settings_.language());
    emit currentLanguageChanged();
    emit currentModelChanged();

    emit logFontChanged();
}

QVariantList AppController::availableLanguages() const {
    QVariantList result;
    for (const LanguageInfo &l : LanguageManager::availableLanguages()) {
        QVariantMap m;
        m[QStringLiteral("code")] = l.code;
        m[QStringLiteral("nativeName")] = l.nativeName;
        result.push_back(m);
    }
    return result;
}

void AppController::setDraftText(const QString &text) {
    if (draftText_ == text) return;
    draftText_ = text;
    emit draftTextChanged();
}

void AppController::clearDraft() { setDraftText(QString()); }

void AppController::setEchoTx(bool on) {
    if (echoTx_ == on) return;
    echoTx_ = on;
    emit echoTxChanged();
}

void AppController::setSendAsHex(bool on) {
    if (sendAsHex_ == on) return;
    sendAsHex_ = on;
    emit sendAsHexChanged();
}

void AppController::setRepeatSendEnabled(bool on) {
    if (repeatEnabled_ == on) return;
    repeatEnabled_ = on;
    if (on && serial_.isOpen())
        repeatTimer_->start(repeatIntervalMs_);
    else
        repeatTimer_->stop();
    emit repeatSendChanged();
}

void AppController::setRepeatIntervalMs(int ms) {
    if (repeatIntervalMs_ == ms) return;
    repeatIntervalMs_ = ms;
    if (repeatTimer_->isActive()) repeatTimer_->start(repeatIntervalMs_);
    emit repeatSendChanged();
}

QByteArray AppController::composeAsciiPayload(const QString &text) const {
    QByteArray bytes = text.toUtf8();
    if (!bytes.endsWith('\n')) bytes += "\r\n";
    return bytes;
}

QByteArray AppController::composeHexPayload(const QString &text) const {
    QString cleaned = text;
    cleaned.remove(QLatin1Char(' '));
    cleaned.remove(QLatin1Char('\n'));
    cleaned.remove(QLatin1Char('\r'));
    cleaned.remove(QLatin1Char('\t'));
    return QByteArray::fromHex(cleaned.toUtf8());
}

void AppController::handleIncomingData(const QByteArray &data) {
    // Longest line this will buffer before flushing it unterminated -- well
    // past any real device log line, so it only ever kicks in for a
    // no-newlines-at-all binary stream.
    static constexpr int kMaxBufferedLine = 8192;

    rxLineBuffer_ += data;

    int start = 0;
    while (true) {
        const int newlineAt = rxLineBuffer_.indexOf('\n', start);
        if (newlineAt < 0) break;
        QByteArray line = rxLineBuffer_.mid(start, newlineAt - start);
        if (line.endsWith('\r')) line.chop(1);
        logManager_.append(LogKind::Rx, line);
        start = newlineAt + 1;
    }
    rxLineBuffer_ = rxLineBuffer_.mid(start);

    if (rxLineBuffer_.size() >= kMaxBufferedLine) {
        flushRxLineBuffer();
    } else if (!rxLineBuffer_.isEmpty()) {
        rxFlushTimer_->start(80);
    } else {
        rxFlushTimer_->stop();
    }
}

void AppController::flushRxLineBuffer() {
    if (rxLineBuffer_.isEmpty()) return;
    QByteArray line = rxLineBuffer_;
    if (line.endsWith('\r')) line.chop(1);
    logManager_.append(LogKind::Rx, line);
    rxLineBuffer_.clear();
}

void AppController::sendLiteral(const QString &text) {
    if (!serial_.isOpen()) {
        logManager_.append(LogKind::Err, tr("Port is not open — click \"Open port\" first.").toUtf8());
        return;
    }
    serial_.write(composeAsciiPayload(text));
    if (echoTx_) logManager_.append(LogKind::Tx, text.toUtf8());
}

bool AppController::openPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits,
                              int flowControl) {
    if (portName.isEmpty()) {
        emit portOpenFailed(tr("Select a serial port first."));
        return false;
    }

    SerialConfig cfg;
    cfg.portName = portName;
    cfg.baudRate = baudRate;
    cfg.dataBits = static_cast<QSerialPort::DataBits>(dataBits);
    cfg.parity = static_cast<QSerialPort::Parity>(parity);
    cfg.stopBits = static_cast<QSerialPort::StopBits>(stopBits);
    cfg.flowControl = static_cast<QSerialPort::FlowControl>(flowControl);

    QString error;
    if (!serial_.open(cfg, &error)) {
        emit portOpenFailed(error);
        return false;
    }

    settings_.setLastSerialConfig(cfg);
    logManager_.append(LogKind::Sys, tr("%1 opened · %2").arg(cfg.portName, serial_.portSummary()).toUtf8());
    if (repeatEnabled_) repeatTimer_->start(repeatIntervalMs_);
    return true;
}

void AppController::closePort() {
    if (!serial_.isOpen()) return;
    serial_.close();
    logManager_.append(LogKind::Sys, tr("Port closed").toUtf8());
}

void AppController::sendManualText() {
    const QString text = draftText_;
    if (text.trimmed().isEmpty()) return;

    if (!serial_.isOpen()) {
        logManager_.append(LogKind::Err, tr("Port is not open — click \"Open port\" first.").toUtf8());
        return;
    }

    if (sendAsHex_) {
        const QByteArray payload = composeHexPayload(text);
        serial_.write(payload);
        if (echoTx_) logManager_.append(LogKind::Tx, payload.toHex(' ').toUpper());
    } else {
        serial_.write(composeAsciiPayload(text));
        if (echoTx_) logManager_.append(LogKind::Tx, text.toUtf8());
    }

    // Repeat-send calls this on every timer tick with the same draftText_ --
    // push() itself no-ops when text already sits at the front of the
    // history, so that doesn't flood the list with one row per tick.
    commandHistoryModel_->push(text);
}

void AppController::activateCommandRow(int row) {
    const DeviceCommand *cmd = commandModel_->commandAt(row);
    if (!cmd || cmd->hasParams()) return;
    sendLiteral(cmd->cmdTemplate);
}

QString AppController::commandNameForRow(int row) const {
    const DeviceCommand *cmd = commandModel_->commandAt(row);
    return cmd ? cmd->name.text() : QString();
}

QVariantList AppController::paramsForRow(int row) const {
    QVariantList result;
    const DeviceCommand *cmd = commandModel_->commandAt(row);
    if (!cmd) return result;
    for (const CommandParam &p : cmd->params) {
        QVariantMap m;
        m[QStringLiteral("key")] = p.key;
        m[QStringLiteral("label")] = p.label.text();
        m[QStringLiteral("hint")] = p.hint;
        m[QStringLiteral("defaultValue")] = p.defaultValue;
        result.push_back(m);
    }
    return result;
}

QString AppController::previewCommand(int row, const QVariantMap &values) const {
    const DeviceCommand *cmd = commandModel_->commandAt(row);
    return cmd ? cmd->resolve(toHash(values)) : QString();
}

void AppController::sendCommandWithParams(int row, const QVariantMap &values) {
    const DeviceCommand *cmd = commandModel_->commandAt(row);
    if (!cmd) return;
    sendLiteral(cmd->resolve(toHash(values)));
}

void AppController::toggleFavorite(int row) { commandModel_->toggleFavorite(row); }

QString AppController::finishWizard(const QString &portName, const QString &modelId) {
    SerialConfig cfg;
    cfg.portName = portName;

    QString error;
    if (!serial_.open(cfg, &error)) return error;

    setCurrentModelId(modelId);
    settings_.setLastSerialConfig(cfg);
    logManager_.append(LogKind::Sys, tr("Wizard finished · %1 opened · %2").arg(portName, modelId).toUtf8());
    emit wizardFinished();
    return QString();
}

QString AppController::suggestedLogBaseName() const {
    QString modelPart = commandModel_->modelId();
    modelPart.replace(QLatin1Char(' '), QLatin1Char('-'));
    if (modelPart.isEmpty()) modelPart = QStringLiteral("device");
    return QStringLiteral("ubibot-%1-%2")
        .arg(modelPart, QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmm")));
}

QString AppController::suggestedLogDirectory() const {
    QString dir = settings_.lastLogDirectory();
    if (dir.isEmpty())
        dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QStringLiteral("/UbiBot");
    return dir;
}

QString AppController::saveLog(const QString &directory, const QString &baseFileName, const QString &format,
                                bool autoRotate) {
    LogFileFormat fmt = LogFileFormat::PlainText;
    QString ext = QStringLiteral(".log");
    if (format == QStringLiteral("csv")) {
        fmt = LogFileFormat::Csv;
        ext = QStringLiteral(".csv");
    } else if (format == QStringLiteral("hex")) {
        fmt = LogFileFormat::HexDump;
        ext = QStringLiteral(".hex.txt");
    }

    const QString path = QDir(directory).filePath(baseFileName + ext);
    QString error;
    if (!logManager_.exportToFile(path, fmt, &error)) return error;

    settings_.setLastLogDirectory(directory);
    settings_.setContinuousLoggingEnabled(autoRotate);
    logManager_.setContinuousLogging(autoRotate, directory, baseFileName);

    emit statusMessage(tr("Log saved to %1").arg(path));
    return QString();
}

QString AppController::checkForLibraryUpdate() const {
    return tr("Bundled command library is %1 — this build checks the local copy only.").arg(library_.version());
}
