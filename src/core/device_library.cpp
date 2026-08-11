#include "core/device_library.h"
#include "core/language_manager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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

}  // namespace

bool DeviceLibrary::loadFromResource(const QString &resourcePath) {
    models_.clear();
    error_.clear();

    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error_ = QStringLiteral("cannot open %1").arg(resourcePath);
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        error_ = parseError.errorString();
        return false;
    }

    const QJsonObject root = doc.object();
    version_ = root.value(QStringLiteral("version")).toString();

    const QJsonArray modelArray = root.value(QStringLiteral("models")).toArray();
    models_.reserve(modelArray.size());
    for (const QJsonValue &modelVal : modelArray) {
        const QJsonObject modelObj = modelVal.toObject();
        DeviceModel model;
        model.id = modelObj.value(QStringLiteral("id")).toString();
        model.description = readLocalized(modelObj.value(QStringLiteral("description")).toObject());

        const QJsonArray cmdArray = modelObj.value(QStringLiteral("commands")).toArray();
        model.commands.reserve(cmdArray.size());
        for (const QJsonValue &cmdVal : cmdArray) {
            const QJsonObject cmdObj = cmdVal.toObject();
            DeviceCommand cmd;
            cmd.group = readLocalized(cmdObj.value(QStringLiteral("group")).toObject());
            cmd.name = readLocalized(cmdObj.value(QStringLiteral("name")).toObject());
            cmd.cmdTemplate = cmdObj.value(QStringLiteral("cmd")).toString();

            const QJsonArray paramArray = cmdObj.value(QStringLiteral("params")).toArray();
            cmd.params.reserve(paramArray.size());
            for (const QJsonValue &paramVal : paramArray) {
                const QJsonObject paramObj = paramVal.toObject();
                CommandParam param;
                param.key = paramObj.value(QStringLiteral("key")).toString();
                param.label = readLocalized(paramObj.value(QStringLiteral("label")).toObject());
                param.hint = paramObj.value(QStringLiteral("hint")).toString();
                param.defaultValue = paramObj.value(QStringLiteral("default")).toString();
                cmd.params.push_back(param);
            }

            model.commands.push_back(cmd);
        }

        models_.push_back(model);
    }

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
