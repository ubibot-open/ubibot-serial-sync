#pragma once

#include "core/serial_manager.h"

#include <QAbstractListModel>
#include <QQmlEngine>

// Backs the serial-port picker (settings panel combo + wizard step 1 list).
// Populated from SerialManager::availablePorts(); refresh() re-queries it.
class PortListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by AppController")

public:
    enum Roles { PortNameRole = Qt::UserRole + 1, DescriptionRole, RecommendedRole, DisplayLabelRole };

    explicit PortListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString portNameAt(int row) const;

private:
    QVector<SerialManager::PortInfo> ports_;
};
