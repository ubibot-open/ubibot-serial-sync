#include "models/batch_command_model.h"
#include "core/settings_store.h"

#include <QUuid>

BatchCommandModel::BatchCommandModel(SettingsStore *settings, QObject *parent)
    : QAbstractListModel(parent), settings_(settings) {
    rows_ = settings_->batchCommands();
}

int BatchCommandModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : rows_.size();
}

QVariant BatchCommandModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rows_.size()) return {};
    const BatchCommand &b = rows_.at(index.row());
    switch (role) {
    case NameRole: return b.name;
    case StepCountRole: return int(b.steps.size());
    case IntervalMsRole: return b.intervalMs;
    }
    return {};
}

QHash<int, QByteArray> BatchCommandModel::roleNames() const {
    return {
        {NameRole, "name"},
        {StepCountRole, "stepCount"},
        {IntervalMsRole, "intervalMs"},
    };
}

QVector<BatchCommandStep> BatchCommandModel::stepsFromVariant(const QVariantList &steps) {
    QVector<BatchCommandStep> result;
    for (const QVariant &v : steps) {
        const QVariantMap m = v.toMap();
        // Keep the original (untrimmed) text if it survives the emptiness
        // check -- trimming here would silently eat meaningful leading/
        // trailing whitespace an ASCII step's author typed on purpose.
        const QString text = m.value(QStringLiteral("text")).toString();
        if (text.trimmed().isEmpty()) continue;
        BatchCommandStep step;
        step.text = text;
        step.isHex = m.value(QStringLiteral("isHex")).toBool();
        step.crcEnabled = m.value(QStringLiteral("crc")).toBool();
        result.push_back(step);
    }
    return result;
}

void BatchCommandModel::addBatchCommand(const QString &name, int intervalMs, const QVariantList &steps) {
    const QString trimmedName = name.trimmed();
    const QVector<BatchCommandStep> cleanedSteps = stepsFromVariant(steps);
    if (trimmedName.isEmpty() || cleanedSteps.isEmpty()) return;

    beginInsertRows(QModelIndex(), rows_.size(), rows_.size());
    BatchCommand b;
    b.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    b.name = trimmedName;
    b.intervalMs = qMax(0, intervalMs);
    b.steps = cleanedSteps;
    rows_.push_back(b);
    endInsertRows();

    settings_->setBatchCommands(rows_);
}

void BatchCommandModel::updateBatchCommand(int row, const QString &name, int intervalMs, const QVariantList &steps) {
    if (row < 0 || row >= rows_.size()) return;
    const QString trimmedName = name.trimmed();
    const QVector<BatchCommandStep> cleanedSteps = stepsFromVariant(steps);
    if (trimmedName.isEmpty() || cleanedSteps.isEmpty()) return;

    BatchCommand &b = rows_[row];
    b.name = trimmedName;
    b.intervalMs = qMax(0, intervalMs);
    b.steps = cleanedSteps;
    emit dataChanged(index(row), index(row));

    settings_->setBatchCommands(rows_);
}

void BatchCommandModel::removeBatchCommand(int row) {
    if (row < 0 || row >= rows_.size()) return;
    beginRemoveRows(QModelIndex(), row, row);
    rows_.remove(row);
    endRemoveRows();

    settings_->setBatchCommands(rows_);
}

QVariantList BatchCommandModel::stepsForRow(int row) const {
    QVariantList result;
    if (row < 0 || row >= rows_.size()) return result;
    for (const BatchCommandStep &step : rows_.at(row).steps) {
        QVariantMap m;
        m[QStringLiteral("text")] = step.text;
        m[QStringLiteral("isHex")] = step.isHex;
        m[QStringLiteral("crc")] = step.crcEnabled;
        result.push_back(m);
    }
    return result;
}

const BatchCommand *BatchCommandModel::commandAt(int row) const {
    if (row < 0 || row >= rows_.size()) return nullptr;
    return &rows_.at(row);
}
