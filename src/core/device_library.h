#pragma once

#include <QHash>
#include <QString>
#include <QVector>

// One piece of text from devices.json, stored as a zh/en pair (that's all
// the device data is localized into today). text() below resolves it to
// whichever single language is currently active -- Chinese for any zh_*
// interface language, English otherwise -- via LanguageManager::pick().
struct LocalizedText {
    QString zh;
    QString en;

    QString text() const;
};

struct CommandParam {
    QString key;            // token name, e.g. "sec" for "<sec>" in the command template
    LocalizedText label;
    QString hint;
    QString defaultValue;
};

struct DeviceCommand {
    LocalizedText group;
    LocalizedText name;
    QString cmdTemplate;    // e.g. "AT+INTERVAL=<sec>"
    QVector<CommandParam> params;

    bool hasParams() const { return !params.isEmpty(); }

    // Substitutes each <key> token with the matching value in `values`
    // (falling back to the param's default, then its hint) and returns the
    // literal string that would be sent over the wire.
    QString resolve(const QHash<QString, QString> &values) const;
};

struct DeviceModel {
    QString id;
    LocalizedText description;
    QVector<DeviceCommand> commands;
};

// Loads the bundled device/command catalog (resources/devices.json) once at
// startup. This is intentionally data, not code: adding a new UbiBot model or
// AT command means editing the JSON, not recompiling.
class DeviceLibrary {
public:
    bool loadFromResource(const QString &resourcePath = QStringLiteral(":/data/devices.json"));

    const QVector<DeviceModel> &models() const { return models_; }
    const DeviceModel *model(const QString &id) const;
    QStringList modelIds() const;
    QString version() const { return version_; }
    int commandCount() const;
    QString errorString() const { return error_; }

private:
    QVector<DeviceModel> models_;
    QString version_;
    QString error_;
};
