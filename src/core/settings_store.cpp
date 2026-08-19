#include "core/settings_store.h"

#include <QFont>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QSettings>
#include <QStyleHints>

namespace {
constexpr auto kLanguage = "app/language";
constexpr auto kPort = "serial/port";
constexpr auto kBaud = "serial/baud";
constexpr auto kDataBits = "serial/dataBits";
constexpr auto kParity = "serial/parity";
constexpr auto kStopBits = "serial/stopBits";
constexpr auto kFlowControl = "serial/flowControl";
constexpr auto kLastModel = "device/lastModel";
constexpr auto kCommandOrder = "send/commandOrder";
constexpr auto kWindowGeometry = "window/geometry";
constexpr auto kLastLogDir = "log/lastDirectory";
constexpr auto kCommandHistory = "send/history";
constexpr auto kContinuousLogging = "log/continuousEnabled";
constexpr auto kLogFontFamily = "log/fontFamily";
constexpr auto kLogFontSize = "log/fontSize";
constexpr auto kSystemFontFamily = "app/fontFamily";
constexpr auto kSystemFontSize = "app/fontSize";
constexpr auto kThemeMode = "app/themeMode";
constexpr auto kCustomTemplates = "send/customTemplates";
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

QStringList SettingsStore::commandOrder() const {
    return QSettings().value(kCommandOrder).toStringList();
}

void SettingsStore::setCommandOrder(const QStringList &order) {
    QSettings().setValue(kCommandOrder, order);
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

QVector<CustomCommandTemplate> SettingsStore::customTemplates() const {
    QVector<CustomCommandTemplate> result;
    const QJsonDocument doc = QJsonDocument::fromJson(QSettings().value(kCustomTemplates).toByteArray());
    for (const QJsonValue &v : doc.array()) {
        const QJsonObject o = v.toObject();
        CustomCommandTemplate t;
        t.id = o.value(QStringLiteral("id")).toString();
        t.name = o.value(QStringLiteral("name")).toString();
        t.content = o.value(QStringLiteral("content")).toString();
        result.push_back(t);
    }
    return result;
}

void SettingsStore::setCustomTemplates(const QVector<CustomCommandTemplate> &templates) {
    QJsonArray arr;
    for (const CustomCommandTemplate &t : templates) {
        QJsonObject o;
        o[QStringLiteral("id")] = t.id;
        o[QStringLiteral("name")] = t.name;
        o[QStringLiteral("content")] = t.content;
        arr.push_back(o);
    }
    QSettings().setValue(kCustomTemplates, QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

QString SettingsStore::logFontFamily() const {
#ifdef Q_OS_WIN
    // Same reasoning (and same family) as systemFontFamily()'s own Windows
    // default below -- Consolas's monospace alignment is nice for a
    // terminal-like hex dump, but Windows' own implicit CJK substitution
    // for it reads noticeably thinner than Microsoft YaHei UI, per user
    // feedback that this pane's Chinese text should match the rest of the
    // app's own default rather than standing out.
    static const QString kPlatformDefault = QStringLiteral("Microsoft YaHei UI");
#else
    static const QString kPlatformDefault = QStringLiteral("Consolas");
#endif
    return QSettings().value(kLogFontFamily, kPlatformDefault).toString();
}

void SettingsStore::setLogFontFamily(const QString &family) {
    QSettings().setValue(kLogFontFamily, family);
}

int SettingsStore::logFontSize() const {
    return QSettings().value(kLogFontSize, 13).toInt();
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
    static const QString kPlatformDefault = [] {
#ifdef Q_OS_WIN
        // Windows' own default UI font (Segoe UI, whatever the locale)
        // doesn't actually contain Chinese glyphs -- every Chinese label
        // falls back to the OS's own implicit CJK substitution instead,
        // which reads noticeably thinner/lower-contrast than Microsoft
        // YaHei UI. That's the same family already used as the first CJK
        // fallback in AppController::buildApplicationFont(), and has
        // shipped with Windows as its own "Chinese UI font" since Vista --
        // just never as the *default* default -- so defaulting straight to
        // it here fixes Chinese rendering without needing a fallback at
        // all. Other platforms keep querying their own default: macOS's
        // San Francisco/PingFang pairing and most Linux desktop themes'
        // Noto set both handle Chinese fine already.
        return QStringLiteral("Microsoft YaHei UI");
#else
        return QGuiApplication::font().family();
#endif
    }();
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
    QSettings s;
    if (s.contains(kThemeMode)) {
        const QString mode = s.value(kThemeMode).toString();
        return mode == QStringLiteral("light") ? mode : QStringLiteral("dark");
    }

    // First run: match the OS's own light/dark preference (Windows'
    // "Choose your color mode", macOS's Appearance setting, or the
    // relevant Linux desktop portal -- whichever this Qt build's platform
    // theme plugin can read), same idea as language()'s system-locale
    // match above. QStyleHints::colorScheme() reports Qt::ColorScheme::
    // Unknown when the platform can't tell it one way or the other (an
    // older/minimal platform plugin, a Linux desktop without portal
    // support, ...) -- that and an explicit Dark both fall back to dark.
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light
        ? QStringLiteral("light")
        : QStringLiteral("dark");
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
