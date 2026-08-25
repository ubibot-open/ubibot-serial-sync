#include "core/device_library.h"
#include "core/language_manager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <utility>

QString LocalizedText::text() const {
    return LanguageManager::instance().pick(zh, en);
}

QString DeviceCommand::resolve(const QHash<QString, QString> &values) const {
    QString out = cmdTemplate;
    for (const CommandParam &p : params) {
        const QString token = QStringLiteral("<%1>").arg(p.key);
        QString value = values.value(p.key);
        if (value.isEmpty()) value = p.defaultValue.isEmpty() ? p.hint : p.defaultValue;
        out.replace(token, value);
    }
    return out;
}

namespace {

LocalizedText readLocalized(const QJsonObject &obj) {
    LocalizedText t;
    t.zh = obj.value(QStringLiteral("zh")).toString();
    t.en = obj.value(QStringLiteral("en")).toString();
    return t;
}

DeviceProtocol readProtocol(const QJsonObject &modelObj) {
    return modelObj.value(QStringLiteral("protocol")).toString() == QStringLiteral("json") ? DeviceProtocol::Json
                                                                                             : DeviceProtocol::At;
}

CommandType readCommandType(const QJsonObject &cmdObj) {
    const QString s = cmdObj.value(QStringLiteral("type")).toString();
    if (s == QStringLiteral("set")) return CommandType::Set;
    if (s == QStringLiteral("action")) return CommandType::Action;
    return CommandType::Query;
}

// See docs/device-json-protocol-schema.md#5 -- string values chosen to
// match Qt's own enum names, restricted to the subset SerialOptions (the
// QML-facing dropdown source) actually offers, so a value read here is
// always resolvable back to a combo-box row.
QSerialPort::Parity readParity(const QString &s) {
    if (s == QStringLiteral("Even")) return QSerialPort::EvenParity;
    if (s == QStringLiteral("Odd")) return QSerialPort::OddParity;
    return QSerialPort::NoParity;
}

QSerialPort::StopBits readStopBits(double n) {
    if (n == 2) return QSerialPort::TwoStop;
    if (n == 1.5) return QSerialPort::OneAndHalfStop;
    return QSerialPort::OneStop;
}

QSerialPort::FlowControl readFlowControl(const QString &s) {
    if (s == QStringLiteral("Hardware")) return QSerialPort::HardwareControl;
    if (s == QStringLiteral("Software")) return QSerialPort::SoftwareControl;
    return QSerialPort::NoFlowControl;
}

QVector<CommandParam> readParams(const QJsonObject &cmdObj) {
    QVector<CommandParam> params;
    const QJsonArray paramArray = cmdObj.value(QStringLiteral("params")).toArray();
    params.reserve(paramArray.size());
    for (const QJsonValue &paramVal : paramArray) {
        const QJsonObject paramObj = paramVal.toObject();
        CommandParam param;
        param.key = paramObj.value(QStringLiteral("key")).toString();
        param.label = readLocalized(paramObj.value(QStringLiteral("label")).toObject());
        param.hint = paramObj.value(QStringLiteral("hint")).toString();
        param.defaultValue = paramObj.value(QStringLiteral("default")).toString();
        params.push_back(param);
    }
    return params;
}

}  // namespace

bool DeviceLibrary::loadFromResource(const QString &resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error_ = QStringLiteral("cannot open %1").arg(resourcePath);
        return false;
    }
    return loadFromJsonText(file.readAll());
}

bool DeviceLibrary::loadFromJsonText(const QByteArray &data) {
    error_.clear();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        error_ = parseError.errorString();
        return false;
    }

    // Parsed into locals first, models_/version_ only overwritten once the
    // whole document has parsed cleanly -- a malformed/truncated download
    // (see DeviceLibraryUpdateClient) must never leave the library half
    // replaced or empty when it already had good data from a previous load.
    QVector<DeviceModel> models;
    const QJsonObject root = doc.object();
    const QString version = root.value(QStringLiteral("version")).toString();

    const QJsonArray modelArray = root.value(QStringLiteral("models")).toArray();
    models.reserve(modelArray.size());
    for (const QJsonValue &modelVal : modelArray) {
        const QJsonObject modelObj = modelVal.toObject();
        DeviceModel model;
        model.id = modelObj.value(QStringLiteral("id")).toString();
        model.protocol = readProtocol(modelObj);
        model.name = readLocalized(modelObj.value(QStringLiteral("name")).toObject());
        model.description = readLocalized(modelObj.value(QStringLiteral("description")).toObject());

        if (modelObj.contains(QStringLiteral("serial"))) {
            const QJsonObject serialObj = modelObj.value(QStringLiteral("serial")).toObject();
            model.hasSerialDefaults = true;
            model.serial.baudRate = serialObj.value(QStringLiteral("baudRate")).toInt(115200);
            model.serial.dataBits =
                static_cast<QSerialPort::DataBits>(serialObj.value(QStringLiteral("dataBits")).toInt(8));
            model.serial.parity = readParity(serialObj.value(QStringLiteral("parity")).toString());
            model.serial.stopBits = readStopBits(serialObj.value(QStringLiteral("stopBits")).toDouble(1));
            model.serial.flowControl = readFlowControl(serialObj.value(QStringLiteral("flowControl")).toString());
        }

        const QJsonArray cmdArray = modelObj.value(QStringLiteral("commands")).toArray();
        model.commands.reserve(cmdArray.size());
        for (const QJsonValue &cmdVal : cmdArray) {
            const QJsonObject cmdObj = cmdVal.toObject();
            DeviceCommand cmd;
            cmd.id = cmdObj.value(QStringLiteral("id")).toString();
            cmd.group = readLocalized(cmdObj.value(QStringLiteral("group")).toObject());
            cmd.name = readLocalized(cmdObj.value(QStringLiteral("name")).toObject());
            cmd.isJsonProtocol = (model.protocol == DeviceProtocol::Json);
            cmd.params = readParams(cmdObj);
            // "type" (query/set/action) is just a descriptive classification
            // -- meaningful for AT commands too, not only JSON ones -- so
            // it's read the same way regardless of protocol.
            cmd.type = readCommandType(cmdObj);

            if (cmd.isJsonProtocol) {
                cmd.needsInput = cmdObj.value(QStringLiteral("needsInput")).toBool();
                const QByteArray encoded = cmdObj.value(QStringLiteral("payloadBase64")).toString().toLatin1();
                cmd.jsonPayload = QString::fromUtf8(QByteArray::fromBase64(encoded));
            } else {
                cmd.cmdTemplate = cmdObj.value(QStringLiteral("cmd")).toString();
                // AT protocol never had an explicit needsInput field --
                // derive it from params so this stays a meaningful,
                // protocol-independent flag on DeviceCommand even though
                // (see device_library.h) nothing branches on it at runtime
                // anymore.
                cmd.needsInput = cmd.hasParams();
            }

            model.commands.push_back(cmd);
        }

        models.push_back(model);
    }

    models_ = std::move(models);
    version_ = version;
    return true;
}

const DeviceModel *DeviceLibrary::model(const QString &id) const {
    for (const DeviceModel &m : models_) {
        if (m.id == id) return &m;
    }
    return models_.isEmpty() ? nullptr : &models_.first();
}

QStringList DeviceLibrary::modelIds() const {
    QStringList ids;
    ids.reserve(models_.size());
    for (const DeviceModel &m : models_) ids.push_back(m.id);
    return ids;
}

int DeviceLibrary::commandCount() const {
    int n = 0;
    for (const DeviceModel &m : models_) n += m.commands.size();
    return n;
}
