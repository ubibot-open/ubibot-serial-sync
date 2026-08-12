#include "models/port_list_model.h"

PortListModel::PortListModel(QObject *parent) : QAbstractListModel(parent) {
    refresh();
}

int PortListModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : ports_.size();
}

QVariant PortListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= ports_.size()) return {};
    const SerialManager::PortInfo &info = ports_.at(index.row());
    switch (role) {
    case PortNameRole: return info.portName;
    case DescriptionRole: return info.description;
    case RecommendedRole: return info.hint == SerialManager::PortHint::Recommended;
    case DisplayLabelRole:
        return info.description.isEmpty() ? info.portName
                                           : QStringLiteral("%1 (%2)").arg(info.portName, info.description);
    }
    return {};
}

QHash<int, QByteArray> PortListModel::roleNames() const {
    return {
        {PortNameRole, "portName"},
        {DescriptionRole, "description"},
        {RecommendedRole, "recommended"},
        {DisplayLabelRole, "displayLabel"},
    };
}

void PortListModel::refresh() {
    beginResetModel();
    ports_ = SerialManager::availablePorts();
    endResetModel();
}

QString PortListModel::portNameAt(int row) const {
    return (row >= 0 && row < ports_.size()) ? ports_.at(row).portName : QString();
}

int PortListModel::indexOfPortName(const QString &portName) const {
    for (int i = 0; i < ports_.size(); ++i) {
        if (ports_.at(i).portName == portName) return i;
    }
    return -1;
}
