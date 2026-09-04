#pragma once

#include "core/serial_manager.h"

#include <QAbstractListModel>
#include <QQmlEngine>

// Backs the serial-port picker (settings panel combo). Populated from
// SerialManager::availablePorts(); refresh() re-queries it.
class PortListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by AppController")

public:
    enum Roles { PortNameRole = Qt::UserRole + 1, DescriptionRole, RecommendedRole, DisplayLabelRole, ChipLabelRole };

    explicit PortListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Re-queries SerialManager::availablePorts() and, only if the result
    // actually differs from what's already loaded, resets the model to it.
    // Returns whether anything changed -- AppController's poll timer (see
    // AppController::refreshPorts()) uses this to skip its own
    // reconcile-the-current-selection pass on every unchanged tick, and a
    // no-op beginResetModel()/endResetModel() pair on an unchanged list
    // would otherwise flicker/rescroll the popup if it happened to be open
    // at the time.
    Q_INVOKABLE bool refresh();
    Q_INVOKABLE QString portNameAt(int row) const;
    // -1 if not present (e.g. unplugged since it was selected). Lets a
    // combo box resolve a remembered port *name* back to a row index after
    // refresh() reshuffles/resizes the list.
    Q_INVOKABLE int indexOfPortName(const QString &portName) const;

private:
    QVector<SerialManager::PortInfo> ports_;
};
