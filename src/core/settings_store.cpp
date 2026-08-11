#include "core/settings_store.h"

#include <QSettings>

namespace {
constexpr auto kLanguage = "app/language";
constexpr auto kPort = "serial/port";
constexpr auto kBaud = "serial/baud";
constexpr auto kDataBits = "serial/dataBits";
constexpr auto kParity = "serial/parity";
constexpr auto kStopBits = "serial/stopBits";
constexpr auto kFlowControl = "serial/flowControl";
constexpr auto kLastModel = "device/lastModel";
constexpr auto kFavoritesGroup = "favorites";
constexpr auto kWindowGeometry = "window/geometry";
constexpr auto kLastLogDir = "log/lastDirectory";
constexpr auto kContinuousLogging = "log/continuousEnabled";
}  // namespace

AppLanguage SettingsStore::language() const {
    const int v = QSettings().value(kLanguage, int(AppLanguage::Bilingual)).toInt();
    if (v == int(AppLanguage::Chinese)) return AppLanguage::Chinese;
    if (v == int(AppLanguage::English)) return AppLanguage::English;
    return AppLanguage::Bilingual;
}

void SettingsStore::setLanguage(AppLanguage lang) {
    QSettings().setValue(kLanguage, int(lang));
}

SerialConfig SettingsStore::lastSerialConfig() const {
    QSettings s;
    SerialConfig cfg;
    cfg.portName = s.value(kPort).toString();
    cfg.baudRate = s.value(kBaud, 115200).toInt();
    cfg.dataBits = static_cast<QSerialPort::DataBits>(s.value(kDataBits, QSerialPort::Data8).toInt());
    cfg.parity = static_cast<QSerialPort::Parity>(s.value(kParity, QSerialPort::NoParity).toInt());
    cfg.stopBits = static_cast<QSerialPort::StopBits>(s.value(kStopBits, QSerialPort::OneStop).toInt());
    cfg.flowControl =
        static_cast<QSerialPort::FlowControl>(s.value(kFlowControl, QSerialPort::NoFlowControl).toInt());
    return cfg;
}

void SettingsStore::setLastSerialConfig(const SerialConfig &cfg) {
    QSettings s;
    s.setValue(kPort, cfg.portName);
    s.setValue(kBaud, cfg.baudRate);
    s.setValue(kDataBits, int(cfg.dataBits));
    s.setValue(kParity, int(cfg.parity));
    s.setValue(kStopBits, int(cfg.stopBits));
    s.setValue(kFlowControl, int(cfg.flowControl));
}

QString SettingsStore::lastModelId() const {
    return QSettings().value(kLastModel).toString();
}

void SettingsStore::setLastModelId(const QString &id) {
    QSettings().setValue(kLastModel, id);
}

bool SettingsStore::isFavorite(const QString &modelId, const QString &commandName) const {
    QSettings s;
    s.beginGroup(kFavoritesGroup);
    const bool fav = s.value(modelId + QLatin1Char('/') + commandName, false).toBool();
    s.endGroup();
    return fav;
}

void SettingsStore::setFavorite(const QString &modelId, const QString &commandName, bool fav) {
    QSettings s;
    s.beginGroup(kFavoritesGroup);
    if (fav)
        s.setValue(modelId + QLatin1Char('/') + commandName, true);
    else
        s.remove(modelId + QLatin1Char('/') + commandName);
    s.endGroup();
}

QByteArray SettingsStore::windowGeometry() const {
    return QSettings().value(kWindowGeometry).toByteArray();
}

void SettingsStore::setWindowGeometry(const QByteArray &geometry) {
    QSettings().setValue(kWindowGeometry, geometry);
}

QString SettingsStore::lastLogDirectory() const {
    return QSettings().value(kLastLogDir).toString();
}

void SettingsStore::setLastLogDirectory(const QString &dir) {
    QSettings().setValue(kLastLogDir, dir);
}

bool SettingsStore::continuousLoggingEnabled() const {
    return QSettings().value(kContinuousLogging, false).toBool();
}

void SettingsStore::setContinuousLoggingEnabled(bool enabled) {
    QSettings().setValue(kContinuousLogging, enabled);
}
