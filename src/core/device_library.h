#pragma once

#include <QHash>
#include <QSerialPort>
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

// See docs/device-json-protocol-schema.md. "At" is the original plain-text
// AT-command protocol (WS1/WS1 Pro/GS1-AL4G1RS/SP1 today); "Json" is
// devices that speak JSON request objects (e.g. {"command":"ReadProduct"}).
enum class DeviceProtocol { At, Json };

// Only meaningful for DeviceProtocol::Json commands. Purely descriptive --
// nothing in code branches on it except UI grouping/labeling -- but see
// needsInput below for the field that actually drives behavior.
enum class CommandType { Query, Set, Action };

struct CommandParam {
    // AT protocol: the <key> token substituted in cmdTemplate at send time
    // (DeviceCommand::resolve). JSON protocol: purely documentation -- see
    // DeviceCommand::needsInput -- typically just the JSON field name
    // that's blank in the payload and needs filling in by hand.
    QString key;
    LocalizedText label;
    QString hint;
    QString defaultValue;
};

struct DeviceCommand {
    QString id;              // stable key for favorites/history; empty for legacy AT commands (falls back to name.zh)
    LocalizedText group;
    LocalizedText name;
    QString cmdTemplate;     // AT protocol only, e.g. "AT+INTERVAL=<sec>"
    QVector<CommandParam> params;

    bool isJsonProtocol = false;      // copied from the owning DeviceModel at load time
    CommandType type = CommandType::Query;
    // AT protocol: true whenever params is non-empty (kept in sync at load
    // time so callers can check this one flag regardless of protocol).
    // JSON protocol: read from devices.json -- originally decided whether a
    // click sent the command immediately (false) or staged it for manual
    // editing (true); the device command library no longer sends anything
    // itself (see AppController::loadCommandIntoDraft), so every command
    // now stages into the manual-send box regardless of this flag. Left in
    // place as documentation for devices.json authors -- a `true` command's
    // payload still carries unresolved `<key>` placeholders that need
    // filling in by hand before sending, `false` doesn't.
    bool needsInput = false;
    QString jsonPayload;     // JSON protocol only: the UTF-8 text decoded from payloadBase64

    // True for a user-authored "My templates" row (CommandListModel merges
    // these in from SettingsStore::customTemplates(), converting each
    // {id, name, content} into one of these with cmdTemplate = content and
    // everything protocol/param-related left at its default) rather than
    // one read from the bundled devices.json. Drives the "edit"/"delete"
    // affordances in CommandLibraryPanel.qml's row delegate in place of the
    // favorite star -- custom templates aren't favoritable, just a flat,
    // searchable list of the user's own quick-send text.
    bool isCustom = false;

    bool hasParams() const { return !params.isEmpty(); }

    // The literal bytes-to-be (as text) for this command, whichever
    // protocol it is -- what AppController::loadCommandIntoDraft() stages
    // into the manual-send box.
    QString wirePayload() const { return isJsonProtocol ? jsonPayload : cmdTemplate; }

    // AT protocol only. Substitutes each <key> token with the matching
    // value in `values` (falling back to the param's default, then its
    // hint) and returns the literal string that would be sent over the
    // wire.
    QString resolve(const QHash<QString, QString> &values) const;
};

// Recommended serial parameters for a DeviceModel. Only meaningful when
// DeviceModel::hasSerialDefaults is true (i.e. the model's JSON actually had
// a "serial" object) -- otherwise these are just default-constructed and
// unused.
struct SerialDefaults {
    qint32 baudRate = 115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
};

struct DeviceModel {
    QString id;
    DeviceProtocol protocol = DeviceProtocol::At;
    LocalizedText name;          // display name; falls back to `id` when empty
    LocalizedText description;
    bool hasSerialDefaults = false;
    SerialDefaults serial;
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
