#include "core/settings_store.h"

#include <QFont>
#include <QGuiApplication>
#include <QLocale>
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
constexpr auto kCommandHistory = "send/history";
constexpr auto kContinuousLogging = "log/continuousEnabled";
constexpr auto kLogFontFamily = "log/fontFamily";
constexpr auto kLogFontSize = "log/fontSize";
constexpr auto kSystemFontFamily = "app/fontFamily";
constexpr auto kSystemFontSize = "app/fontSize";
constexpr auto kThemeMode = "app/themeMode";
}  // namespace

QString SettingsStore::language() const {
    QSettings s;
    if (s.contains(kLanguage)) return s.value(kLanguage).toString();

    // First run: match the system locale's language against what we ship
    // ("zh_TW" and "zh_CN" both fall back to the "zh_CN" entry, say), else
    // fall back to English.
    const QString sysLanguage = QLocale::system().name().section(QLatin1Char('_'), 0, 0);
    for (const LanguageInfo &lang : LanguageManager::availableLanguages()) {
        if (lang.code.section(QLatin1Char('_'), 0, 0) == sysLanguage) return lang.code;
    }
    return QStringLiteral("en");
}

void SettingsStore::setLanguage(const QString &code) {
    QSettings().setValue(kLanguage, code);
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

QStringList SettingsStore::commandHistory() const {
    return QSettings().value(kCommandHistory).toStringList();
}

void SettingsStore::setCommandHistory(const QStringList &entries) {
    QSettings().setValue(kCommandHistory, entries);
}

bool SettingsStore::continuousLoggingEnabled() const {
    return QSettings().value(kContinuousLogging, false).toBool();
}

void SettingsStore::setContinuousLoggingEnabled(bool enabled) {
    QSettings().setValue(kContinuousLogging, enabled);
}

QString SettingsStore::logFontFamily() const {
    return QSettings().value(kLogFontFamily, QStringLiteral("Consolas")).toString();
}

void SettingsStore::setLogFontFamily(const QString &family) {
    QSettings().setValue(kLogFontFamily, family);
}

int SettingsStore::logFontSize() const {
    return QSettings().value(kLogFontSize, 12).toInt();
}

void SettingsStore::setLogFontSize(int pixelSize) {
    QSettings().setValue(kLogFontSize, pixelSize);
}

QString SettingsStore::systemFontFamily() const {
    // Captured once, the first time anything asks -- which happens in
    // main.cpp before QGuiApplication::setFont() is ever called (see
    // AppController::buildApplicationFont()) -- so this stays the
    // platform's true original default even after the user picks a custom
    // family and later hits "Restore defaults", at which point
    // QGuiApplication::font() itself no longer holds that original value.
    static const QString kPlatformDefault = QGuiApplication::font().family();
    return QSettings().value(kSystemFontFamily, kPlatformDefault).toString();
}

void SettingsStore::setSystemFontFamily(const QString &family) {
    QSettings().setValue(kSystemFontFamily, family);
}

int SettingsStore::systemFontSize() const {
    return QSettings().value(kSystemFontSize, 13).toInt();
}

void SettingsStore::setSystemFontSize(int pixelSize) {
    QSettings().setValue(kSystemFontSize, pixelSize);
}

QString SettingsStore::themeMode() const {
    const QString mode = QSettings().value(kThemeMode, QStringLiteral("dark")).toString();
    return mode == QStringLiteral("light") ? mode : QStringLiteral("dark");
}

void SettingsStore::setThemeMode(const QString &mode) {
    QSettings().setValue(kThemeMode, mode == QStringLiteral("light") ? QStringLiteral("light") : QStringLiteral("dark"));
}

void SettingsStore::resetDisplayPreferences() {
    QSettings s;
    s.remove(kLanguage);
    s.remove(kLogFontFamily);
    s.remove(kLogFontSize);
    s.remove(kSystemFontFamily);
    s.remove(kSystemFontSize);
    s.remove(kThemeMode);
}
