#include "models/log_list_model.h"

namespace {
QString colorForKind(LogKind k) {
    switch (k) {
    case LogKind::Tx: return QStringLiteral("#2c455d");
    case LogKind::Rx: return QStringLiteral("#1d1f20");
    case LogKind::Sys: return QStringLiteral("#808080");
    case LogKind::Err: return QStringLiteral("#aa3333");
    }
    return QStringLiteral("#1d1f20");
}
}  // namespace

LogListModel::LogListModel(LogManager *manager, QObject *parent)
    : QAbstractListModel(parent), manager_(manager) {
    connect(manager_, &LogManager::rowAboutToBeRemoved, this,
            [this] { beginRemoveRows(QModelIndex(), 0, 0); });
    connect(manager_, &LogManager::rowRemoved, this, [this] { endRemoveRows(); });
    connect(manager_, &LogManager::entryAdded, this, [this] {
        const int row = manager_->entries().size() - 1;
        beginInsertRows(QModelIndex(), row, row);
        endInsertRows();
        emit lineCountChanged();
    });
    connect(manager_, &LogManager::cleared, this, [this] {
        beginResetModel();
        endResetModel();
        emit lineCountChanged();
    });
    connect(manager_, &LogManager::countersChanged, this, &LogListModel::countersChanged);
}

int LogListModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : manager_->entries().size();
}

QVariant LogListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= manager_->entries().size()) return {};
    const LogEntry &e = manager_->entries().at(index.row());
    switch (role) {
    case TimeRole: return showTimestamp_ ? e.time.toString(QStringLiteral("HH:mm:ss")) : QString();
    case DirRole:
        switch (e.kind) {
        case LogKind::Tx: return QStringLiteral("TX");
        case LogKind::Rx: return QStringLiteral("RX");
        case LogKind::Sys: return QStringLiteral("SYS");
        case LogKind::Err: return QStringLiteral("ERR");
        }
        return QString();
    case TextRole: return (hexMode_ && e.kind == LogKind::Rx) ? e.hexText() : e.asciiText();
    case ColorRole: return colorForKind(e.kind);
    }
    return {};
}

QHash<int, QByteArray> LogListModel::roleNames() const {
    return {
        {TimeRole, "time"},
        {DirRole, "dir"},
        {TextRole, "text"},
        {ColorRole, "color"},
    };
}

void LogListModel::setHexMode(bool hex) {
    if (hexMode_ == hex) return;
    hexMode_ = hex;
    emit hexModeChanged();
    if (!manager_->entries().isEmpty())
        emit dataChanged(index(0), index(manager_->entries().size() - 1), {TextRole});
}

void LogListModel::setShowTimestamp(bool show) {
    if (showTimestamp_ == show) return;
    showTimestamp_ = show;
    emit showTimestampChanged();
    if (!manager_->entries().isEmpty())
        emit dataChanged(index(0), index(manager_->entries().size() - 1), {TimeRole});
}

int LogListModel::lineCount() const { return manager_->entries().size(); }
qint64 LogListModel::rxBytes() const { return manager_->rxBytes(); }
qint64 LogListModel::txBytes() const { return manager_->txBytes(); }

void LogListModel::clear() { manager_->clear(); }
