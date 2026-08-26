#include "app/app_controller.h"

#include "core/crc16.h"
#include "core/env_config.h"
#include "core/language_manager.h"
#include "core/self_update_installer.h"
#include "models/batch_command_model.h"
#include "models/command_history_model.h"
#include "models/command_list_model.h"
#include "models/log_list_model.h"
#include "models/port_list_model.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QTimer>
#include <QVersionNumber>

namespace {
QHash<QString, QString> toHash(const QVariantMap &values) {
    QHash<QString, QString> h;
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) h.insert(it.key(), it.value().toString());
    return h;
}
}  // namespace

AppController::AppController(QObject *parent) : QObject(parent) {
    // Prefer the last successfully downloaded library (see
    // downloadLibraryUpdate() below) over the one baked into the binary at
    // compile time -- falls back to the bundled resources/devices.json
    // whenever nothing's been downloaded yet, or the cached text somehow
    // fails to parse (corrupt QSettings value, older cache from an
    // incompatible schema, ...), so a bad cache can never leave the app with
    // no command library at all.
    const QString cachedJson = settings_.cachedLibraryJson();
    if (cachedJson.isEmpty() || !library_.loadFromJsonText(cachedJson.toUtf8())) {
        library_.loadFromResource();
    }
    LanguageManager::instance().setLanguage(settings_.language());

    logModel_ = new LogListModel(&logManager_, this);
    commandModel_ = new CommandListModel(&library_, &settings_, this);
    portListModel_ = new PortListModel(this);
    commandHistoryModel_ = new CommandHistoryModel(&settings_, this);
    batchCommandModel_ = new BatchCommandModel(&settings_, this);

    repeatTimer_ = new QTimer(this);
    connect(repeatTimer_, &QTimer::timeout, this, &AppController::sendManualText);

    batchTimer_ = new QTimer(this);
    connect(batchTimer_, &QTimer::timeout, this, &AppController::sendNextBatchStep);

    rxFlushTimer_ = new QTimer(this);
    rxFlushTimer_->setSingleShot(true);
    connect(rxFlushTimer_, &QTimer::timeout, this, &AppController::flushRxLineBuffer);

    connect(&serial_, &SerialManager::dataReceived, this, &AppController::handleIncomingData);
    connect(&serial_, &SerialManager::errorOccurred, this,
            [this](const QString &msg) { logManager_.append(LogKind::Err, msg.toUtf8()); });
    connect(&serial_, &SerialManager::opened, this, [this](const SerialConfig &) { emit connectionChanged(); });
    connect(&serial_, &SerialManager::closed, this, [this] {
        repeatTimer_->stop();
        stopBatchCommand();
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

    libraryUpdateClient_ =
        new DeviceLibraryUpdateClient(EnvConfig::instance().value(QStringLiteral("DEVICE_LIBRARY_API_BASE_URL")),
                                       EnvConfig::instance().value(QStringLiteral("DEVICE_LIBRARY_API_KEY")), this);

    connect(libraryUpdateClient_, &DeviceLibraryUpdateClient::checkFinished, this,
            [this](bool ok, bool updateAvailable, const QString &remoteVersion, const QString &minAppVersion,
                   const QString &message, const QString &error) {
                settings_.setLastLibraryCheckAt(QDateTime::currentDateTimeUtc());
                remoteLibraryVersion_ = remoteVersion;

                // A newer library that requires a newer app than this build
                // (docs/device-library-update-protocol.md#4/#6) is reported
                // but not downloadable -- surfaced as its own message rather
                // than silently either offering a download that would break
                // something, or hiding that an update exists at all.
                const bool appTooOld = !minAppVersion.isEmpty() &&
                    QVersionNumber::fromString(QCoreApplication::applicationVersion()) <
                        QVersionNumber::fromString(minAppVersion);

                if (!ok) {
                    libraryUpdateState_ = QStringLiteral("error");
                    libraryUpdateMessage_ = error;
                    libraryUpdateAvailable_ = false;
                } else if (!updateAvailable) {
                    libraryUpdateState_ = QStringLiteral("upToDate");
                    libraryUpdateMessage_ = tr("Command library is up to date (%1).").arg(library_.version());
                    libraryUpdateAvailable_ = false;
                } else if (appTooOld) {
                    libraryUpdateState_ = QStringLiteral("error");
                    libraryUpdateMessage_ =
                        tr("Version %1 is available, but requires app version %2 or later -- please update the app first.")
                            .arg(remoteVersion, minAppVersion);
                    libraryUpdateAvailable_ = false;
                } else {
                    libraryUpdateState_ = QStringLiteral("updateAvailable");
                    libraryUpdateMessage_ =
                        message.isEmpty() ? tr("Version %1 is available.").arg(remoteVersion) : message;
                    libraryUpdateAvailable_ = true;
                }
                emit libraryUpdateStateChanged();
            });

    connect(libraryUpdateClient_, &DeviceLibraryUpdateClient::fetchFinished, this,
            [this](bool ok, const QString &version, const QByteArray &rawJson, const QString &error) {
                if (!ok || !library_.loadFromJsonText(rawJson)) {
                    libraryUpdateState_ = QStringLiteral("error");
                    libraryUpdateMessage_ =
                        !error.isEmpty() ? error : tr("Downloaded data could not be applied: %1").arg(library_.errorString());
                    emit libraryUpdateStateChanged();
                    return;
                }

                settings_.setCachedLibraryJson(QString::fromUtf8(rawJson));
                commandModel_->reload();
                emit libraryChanged();
                if (library_.model(commandModel_->modelId()) == nullptr) {
                    // The previously selected model no longer exists in the
                    // new library (removed upstream) -- fall back to
                    // whatever the new library's first model is, same as a
                    // fresh install with no lastModelId at all.
                    setCurrentModelId(library_.modelIds().value(0));
                }

                libraryUpdateState_ = QStringLiteral("upToDate");
                libraryUpdateMessage_ = tr("Updated to %1.").arg(version);
                libraryUpdateAvailable_ = false;
                emit libraryUpdateStateChanged();
            });

    // Deliberately its own EnvConfig keys (not shared with
    // DEVICE_LIBRARY_API_BASE_URL/_API_KEY above) -- see
    // docs/app-self-update.md#1. Points at the API root (e.g.
    // ".../api"), not a product-specific path, since
    // SoftwareUpdateClient::checkForUpdate() appends "/software/list"
    // itself.
    softwareUpdateClient_ =
        new SoftwareUpdateClient(EnvConfig::instance().value(QStringLiteral("APP_UPDATE_API_BASE_URL")),
                                  EnvConfig::instance().value(QStringLiteral("APP_UPDATE_API_KEY")), this);

    connect(softwareUpdateClient_, &SoftwareUpdateClient::checkFinished, this,
            [this](bool ok, bool updateAvailable, bool appTooOld, const QString &version, const QString &downloadUrl,
                   qint64 /*fileSize*/, const QString &changelog, bool forceUpdate, const QString &minRequiredVersion,
                   const QString &sha256, const QString &error) {
                remoteAppVersion_ = version;
                appUpdateForced_ = forceUpdate;
                pendingUpdateDownloadUrl_ = downloadUrl;
                pendingUpdateSha256_ = sha256;
                pendingUpdateVersion_ = version;

                if (!ok) {
                    appUpdateState_ = QStringLiteral("error");
                    appUpdateMessage_ = error;
                    appUpdateAvailable_ = false;
                } else if (!updateAvailable) {
                    appUpdateState_ = QStringLiteral("upToDate");
                    appUpdateMessage_ = tr("You're on the latest version (%1).").arg(appVersion());
                    appUpdateAvailable_ = false;
                } else if (appTooOld) {
                    appUpdateState_ = QStringLiteral("error");
                    appUpdateMessage_ =
                        tr("Version %1 is available, but requires updating from version %2 or later first -- "
                           "please reinstall the app before trying again.")
                            .arg(version, minRequiredVersion);
                    appUpdateAvailable_ = false;
                } else {
                    appUpdateState_ = QStringLiteral("updateAvailable");
                    appUpdateMessage_ = changelog.isEmpty() ? tr("Version %1 is available.").arg(version) : changelog;
                    appUpdateAvailable_ = true;
                }
                emit appUpdateStateChanged();
            });

    connect(softwareUpdateClient_, &SoftwareUpdateClient::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                appUpdateProgress_ = total > 0 ? double(received) / double(total) : 0.0;
                emit appUpdateProgressChanged();
            });

    connect(softwareUpdateClient_, &SoftwareUpdateClient::downloadFinished, this,
            [this](bool ok, const QString &filePath, const QString &error) {
                if (!ok) {
                    appUpdateState_ = QStringLiteral("error");
                    appUpdateMessage_ = error;
                    emit appUpdateStateChanged();
                    return;
                }
                applyDownloadedAppUpdate(filePath);
            });
}

AppController::~AppController() = default;

// Reads back QCoreApplication::applicationVersion() (set in main.cpp from
// the same APP_VERSION compile definition) rather than hardcoding its own
// copy of the string -- see CMakeLists.txt's top-of-file comment for where
// that actually comes from.
QString AppController::appVersion() const { return QCoreApplication::applicationVersion(); }
QString AppController::qtVersion() const { return QStringLiteral(QT_VERSION_STR); }
QString AppController::buildTime() const { return QStringLiteral(APP_BUILD_TIME); }

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

QString AppController::systemFontFamily() const { return settings_.systemFontFamily(); }

void AppController::setSystemFontFamily(const QString &family) {
    if (settings_.systemFontFamily() == family) return;
    settings_.setSystemFontFamily(family);
    QGuiApplication::setFont(buildApplicationFont(family, settings_.systemFontSize()));
    emit systemFontChanged();
}

int AppController::systemFontSize() const { return settings_.systemFontSize(); }

void AppController::setSystemFontSize(int pixelSize) {
    if (settings_.systemFontSize() == pixelSize) return;
    settings_.setSystemFontSize(pixelSize);
    QGuiApplication::setFont(buildApplicationFont(settings_.systemFontFamily(), pixelSize));
    emit systemFontChanged();
}

QFont AppController::buildApplicationFont(const QString &family, int pixelSize) {
    QFont font = QGuiApplication::font();
    QStringList families;
    if (!family.isEmpty()) families << family;
    families << QStringLiteral("Microsoft YaHei UI") << QStringLiteral("Microsoft YaHei")
             << QStringLiteral("PingFang SC") << QStringLiteral("Noto Sans CJK SC");
    font.setFamilies(families);
    if (pixelSize > 0) font.setPixelSize(pixelSize);
    return font;
}

QStringList AppController::availableFontFamilies() const { return QFontDatabase::families(); }

QString AppController::themeMode() const { return settings_.themeMode(); }

void AppController::setThemeMode(const QString &mode) {
    if (settings_.themeMode() == mode) return;
    settings_.setThemeMode(mode);
    emit themeModeChanged();
}

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

    // Visually restore the process-wide default too, not just the persisted
    // value -- otherwise the app keeps rendering the customized font until
    // restart even though settings_.systemFontFamily()/systemFontSize() now
    // report the built-in default again.
    QGuiApplication::setFont(buildApplicationFont(settings_.systemFontFamily(), settings_.systemFontSize()));

    emit logFontChanged();
    emit systemFontChanged();
    emit themeModeChanged();
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

void AppController::setSelectedPortName(const QString &name) {
    if (selectedPortName_ == name) return;
    selectedPortName_ = name;
    emit selectedPortNameChanged();
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

void AppController::setCrcEnabled(bool on) {
    if (crcEnabled_ == on) return;
    crcEnabled_ = on;
    emit crcEnabledChanged();
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

QByteArray AppController::composeAsciiPayload(const QString &text, bool crc) const {
    QByteArray bytes = text.toUtf8();
    // Captured before any CRC bytes are appended below -- otherwise a CRC
    // byte that happens to equal '\n' (0x0A) would make this wrongly think
    // the user's own text already ended in a newline and skip adding the
    // real "\r\n" terminator.
    const bool hasTerminator = bytes.endsWith('\n');
    if (crc) bytes += Crc16::modbusBytes(bytes);
    if (!hasTerminator) bytes += "\r\n";
    return bytes;
}

QByteArray AppController::composeHexPayload(const QString &text, bool crc) const {
    QString cleaned = text;
    cleaned.remove(QLatin1Char(' '));
    cleaned.remove(QLatin1Char('\n'));
    cleaned.remove(QLatin1Char('\r'));
    cleaned.remove(QLatin1Char('\t'));
    QByteArray bytes = QByteArray::fromHex(cleaned.toUtf8());
    if (crc) bytes += Crc16::modbusBytes(bytes);
    return bytes;
}

QString AppController::crcEchoSuffix(const QByteArray &data, bool crc) const {
    if (!crc) return QString();
    return QStringLiteral(" [CRC %1]").arg(QString::fromLatin1(Crc16::modbusBytes(data).toHex(' ').toUpper()));
}

void AppController::handleIncomingData(const QByteArray &data) {
    // Longest line this will buffer before flushing it unterminated -- well
    // past any real device log line, so it only ever kicks in for a
    // no-newlines-at-all binary stream.
    static constexpr int kMaxBufferedLine = 8192;

    rxLineBuffer_ += data;

    // Collected and handed to LogManager as one appendBatch() call instead
    // of one logManager_.append() per line -- a device dumping a large
    // stored log can hand this hundreds of thousands of lines in a single
    // burst (spread across many dataReceived calls, or occasionally all at
    // once if the OS buffered heavily before this had a chance to read),
    // and appending them one at a time meant one full model-update +
    // data-monitor-document cycle per line. Individually cheap, but
    // multiplied across that many lines -- and with an O(document size)
    // cost buried in evicting the oldest one once the scrollback is at
    // capacity -- that was slow enough to freeze the UI outright. See
    // LogManager::appendBatch()/LogListModel's entriesAppended handler.
    QList<QByteArray> lines;
    int start = 0;
    while (true) {
        const int newlineAt = rxLineBuffer_.indexOf('\n', start);
        if (newlineAt < 0) break;
        QByteArray line = rxLineBuffer_.mid(start, newlineAt - start);
        if (line.endsWith('\r')) line.chop(1);
        lines.push_back(line);
        start = newlineAt + 1;
    }
    if (!lines.isEmpty()) logManager_.appendBatch(LogKind::Rx, lines);
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
        const QByteArray payload = composeHexPayload(text, crcEnabled_);
        serial_.write(payload);
        if (echoTx_) logManager_.append(LogKind::Tx, payload.toHex(' ').toUpper());
    } else {
        serial_.write(composeAsciiPayload(text, crcEnabled_));
        if (echoTx_) logManager_.append(LogKind::Tx, (text + crcEchoSuffix(text.toUtf8(), crcEnabled_)).toUtf8());
    }

    // Repeat-send calls this on every timer tick with the same draftText_ --
    // push() itself no-ops when text already sits at the front of the
    // history, so that doesn't flood the list with one row per tick.
    commandHistoryModel_->push(text);
}

void AppController::loadCommandIntoDraft(int row) {
    const DeviceCommand *cmd = commandModel_->commandAt(row);
    if (!cmd) return;
    setDraftText(cmd->wirePayload());
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

void AppController::loadCommandWithParamsIntoDraft(int row, const QVariantMap &values) {
    const DeviceCommand *cmd = commandModel_->commandAt(row);
    if (!cmd) return;
    setDraftText(cmd->resolve(toHash(values)));
}

void AppController::moveCommandRow(int from, int to) { commandModel_->moveRow(from, to); }

void AppController::addBatchCommand(const QString &name, int intervalMs, const QVariantList &steps) {
    batchCommandModel_->addBatchCommand(name, intervalMs, steps);
}

void AppController::updateBatchCommand(int row, const QString &name, int intervalMs, const QVariantList &steps) {
    batchCommandModel_->updateBatchCommand(row, name, intervalMs, steps);
}

void AppController::removeBatchCommand(int row) {
    if (row == batchRunningRow_) {
        // Deleting the batch that's actively sending -- stop it cleanly
        // first rather than leaving batchTimer_ ticking against a snapshot
        // whose source row is about to disappear.
        stopBatchCommand();
    } else if (batchRunningRow_ >= 0 && row < batchRunningRow_) {
        // A row *below* the running one shifts up by one once removed --
        // batchRunningRow_ has to follow it so the dialog keeps
        // highlighting the same logical batch, not whichever row happens
        // to land at the old index afterward.
        --batchRunningRow_;
        emit batchStateChanged();
    }
    batchCommandModel_->removeBatchCommand(row);
}

QVariantList AppController::stepsForBatchRow(int row) const { return batchCommandModel_->stepsForRow(row); }

void AppController::startBatchCommand(int row) {
    const BatchCommand *bc = batchCommandModel_->commandAt(row);
    if (!bc || bc->steps.isEmpty()) return;

    if (!serial_.isOpen()) {
        logManager_.append(LogKind::Err, tr("Port is not open — click \"Open port\" first.").toUtf8());
        return;
    }

    // Only one batch runs at a time -- starting a new one (or re-starting
    // the same one) cleanly stops whatever's already in flight first.
    stopBatchCommand();

    batchQueue_ = *bc;
    batchStepIndex_ = 0;
    batchRunningRow_ = row;
    logManager_.append(
        LogKind::Sys,
        tr("Batch \"%1\" started — %2 step(s).").arg(batchQueue_.name).arg(batchQueue_.steps.size()).toUtf8());
    emit batchStateChanged();

    // Send the first step right away rather than waiting a full interval to
    // see anything happen; sendNextBatchStep() below only needs to cover
    // the steps after it.
    sendNextBatchStep();
    if (batchRunningRow_ >= 0 && batchStepIndex_ < batchQueue_.steps.size())
        batchTimer_->start(qMax(0, batchQueue_.intervalMs));
}

void AppController::sendNextBatchStep() {
    if (batchRunningRow_ < 0 || batchStepIndex_ >= batchQueue_.steps.size()) {
        stopBatchCommand();
        return;
    }
    if (!serial_.isOpen()) {
        stopBatchCommand();
        return;
    }

    const BatchCommandStep &step = batchQueue_.steps.at(batchStepIndex_);
    if (step.isHex) {
        const QByteArray payload = composeHexPayload(step.text, step.crcEnabled);
        serial_.write(payload);
        if (echoTx_) logManager_.append(LogKind::Tx, payload.toHex(' ').toUpper());
    } else {
        serial_.write(composeAsciiPayload(step.text, step.crcEnabled));
        if (echoTx_)
            logManager_.append(LogKind::Tx, (step.text + crcEchoSuffix(step.text.toUtf8(), step.crcEnabled)).toUtf8());
    }
    ++batchStepIndex_;
    emit batchStateChanged();

    if (batchStepIndex_ >= batchQueue_.steps.size()) {
        logManager_.append(LogKind::Sys, tr("Batch \"%1\" finished.").arg(batchQueue_.name).toUtf8());
        stopBatchCommand();
    }
}

void AppController::stopBatchCommand() {
    if (batchRunningRow_ < 0) return;
    // Only log an explicit "stopped" line when this cuts a run short --
    // sendNextBatchStep() above already logs its own "finished" line right
    // before calling this on natural completion, so this would otherwise
    // double up on every normal run.
    if (batchStepIndex_ < batchQueue_.steps.size())
        logManager_.append(LogKind::Sys, tr("Batch \"%1\" stopped.").arg(batchQueue_.name).toUtf8());
    batchTimer_->stop();
    batchRunningRow_ = -1;
    batchStepIndex_ = 0;
    batchQueue_ = BatchCommand();
    emit batchStateChanged();
}

QString AppController::batchProgressText() const {
    if (batchRunningRow_ < 0) return QString();
    return tr("%1 / %2").arg(batchStepIndex_).arg(batchQueue_.steps.size());
}

void AppController::addCustomTemplate(const QString &name, const QString &content) {
    commandModel_->addCustomTemplate(name, content);
}

void AppController::updateCustomTemplate(int row, const QString &name, const QString &content) {
    commandModel_->updateCustomTemplate(row, name, content);
}

void AppController::removeCustomTemplate(int row) { commandModel_->removeCustomTemplate(row); }

QVariantMap AppController::serialDefaultsForModel(const QString &modelId) const {
    const DeviceModel *m = library_.model(modelId);
    QVariantMap result;
    result[QStringLiteral("present")] = m && m->hasSerialDefaults;
    if (!m || !m->hasSerialDefaults) return result;
    result[QStringLiteral("baudRate")] = m->serial.baudRate;
    result[QStringLiteral("dataBits")] = int(m->serial.dataBits);
    result[QStringLiteral("parity")] = int(m->serial.parity);
    result[QStringLiteral("stopBits")] = int(m->serial.stopBits);
    result[QStringLiteral("flowControl")] = int(m->serial.flowControl);
    return result;
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

void AppController::checkForLibraryUpdate() {
    if (!libraryUpdateClient_->isConfigured()) {
        libraryUpdateState_ = QStringLiteral("error");
        libraryUpdateMessage_ = tr("No update server configured (.env) -- using the bundled command library only.");
        libraryUpdateAvailable_ = false;
        emit libraryUpdateStateChanged();
        return;
    }

    libraryUpdateState_ = QStringLiteral("checking");
    libraryUpdateMessage_ = tr("Checking for updates…");
    emit libraryUpdateStateChanged();
    libraryUpdateClient_->checkForUpdate(library_.version());
}

void AppController::downloadLibraryUpdate() {
    if (!libraryUpdateAvailable_) return;  // nothing to apply -- see libraryUpdateAvailable's own doc comment
    libraryUpdateState_ = QStringLiteral("downloading");
    libraryUpdateMessage_ = tr("Downloading…");
    emit libraryUpdateStateChanged();
    libraryUpdateClient_->fetchLatest();
}

void AppController::checkForAppUpdate() {
    if (!softwareUpdateClient_->isConfigured()) {
        appUpdateState_ = QStringLiteral("error");
        appUpdateMessage_ = tr("No update server configured (.env).");
        appUpdateAvailable_ = false;
        emit appUpdateStateChanged();
        return;
    }

    appUpdateState_ = QStringLiteral("checking");
    appUpdateMessage_ = tr("Checking for updates…");
    emit appUpdateStateChanged();
    softwareUpdateClient_->checkForUpdate(appVersion());
}

void AppController::installAppUpdate() {
    if (!appUpdateAvailable_) return;  // nothing to apply -- see appUpdateAvailable's own doc comment

    const QString tempDir =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/UbiBotSerialAssistant-update");
    QDir().mkpath(tempDir);
    const QString zipPath = tempDir + QStringLiteral("/update.zip");

    appUpdateState_ = QStringLiteral("downloading");
    appUpdateMessage_ = tr("Downloading update…");
    appUpdateProgress_ = 0.0;
    emit appUpdateStateChanged();
    emit appUpdateProgressChanged();
    softwareUpdateClient_->download(pendingUpdateDownloadUrl_, zipPath);
}

void AppController::cancelAppUpdateDownload() {
    softwareUpdateClient_->cancelDownload();
    if (appUpdateState_ == QStringLiteral("downloading")) {
        appUpdateState_ = QStringLiteral("updateAvailable");
        appUpdateMessage_ = tr("Download cancelled.");
        emit appUpdateStateChanged();
    }
}

void AppController::applyDownloadedAppUpdate(const QString &zipPath) {
#ifdef Q_OS_WIN
    // Verified before anything else touches the file -- a bad/tampered
    // download must never reach extraction, let alone the install
    // directory. sha256 is presently almost always empty in practice (see
    // docs/app-self-update.md#2), in which case this is skipped entirely --
    // same "best-effort, not a hard requirement" stance as the device-
    // library updater's own checksum handling.
    if (!pendingUpdateSha256_.isEmpty()) {
        const QString actual = SelfUpdateInstaller::sha256OfFile(zipPath);
        if (actual.compare(pendingUpdateSha256_, Qt::CaseInsensitive) != 0) {
            appUpdateState_ = QStringLiteral("error");
            appUpdateMessage_ = tr("Downloaded file failed checksum verification -- update not applied.");
            emit appUpdateStateChanged();
            return;
        }
    }

    appUpdateState_ = QStringLiteral("installing");
    appUpdateMessage_ = tr("Preparing update…");
    emit appUpdateStateChanged();

    const QString exeName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    QString stagingDir;
    const SelfUpdateInstaller::Result extractResult =
        SelfUpdateInstaller::extractAndValidate(zipPath, exeName, stagingDir);
    if (!extractResult.ok) {
        appUpdateState_ = QStringLiteral("error");
        appUpdateMessage_ = extractResult.error;
        emit appUpdateStateChanged();
        return;
    }

    QString scriptError;
    const QString scriptPath =
        SelfUpdateInstaller::writeUpdateScript(stagingDir, QCoreApplication::applicationDirPath(), exeName, zipPath,
                                                QCoreApplication::applicationPid(), scriptError);
    if (scriptPath.isEmpty()) {
        appUpdateState_ = QStringLiteral("error");
        appUpdateMessage_ = scriptError;
        emit appUpdateStateChanged();
        return;
    }

    if (!SelfUpdateInstaller::launchDetached(scriptPath)) {
        appUpdateState_ = QStringLiteral("error");
        appUpdateMessage_ = tr("Failed to launch the updater script.");
        emit appUpdateStateChanged();
        return;
    }

    // The helper script is now waiting on this process's PID -- the sooner
    // this exits, the sooner it can proceed. Nothing past this point runs;
    // there's no "success" state to show, the window just closes.
    appUpdateMessage_ = tr("Restarting to apply update…");
    emit appUpdateStateChanged();
    QCoreApplication::quit();
#else
    Q_UNUSED(zipPath);
    appUpdateState_ = QStringLiteral("error");
    appUpdateMessage_ =
        tr("Self-update isn't supported on this platform yet -- please download the new version manually.");
    emit appUpdateStateChanged();
#endif
}
