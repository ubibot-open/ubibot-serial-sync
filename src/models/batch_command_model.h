#pragma once

#include "core/batch_command.h"

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QVariantList>

class SettingsStore;

// Flat list of the user's saved "Batch commands" (CommandLibraryPanel.qml's
// own batch dialog) -- CRUD only, no search/reorder (a handful of entries
// at most; mirrors CommandListModel's own "My templates" CRUD for the same
// reasoning). AppController owns the actual send engine (its own QTimer,
// wired to serial_/logManager_) since this model has no access to either;
// this model is purely the persisted list plus the roles the dialog's
// ListView renders.
class BatchCommandModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by AppController")

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        StepCountRole,
        IntervalMsRole,
    };

    explicit BatchCommandModel(SettingsStore *settings, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // CRUD for the batch dialog's add/edit/delete. `steps` is a QVariantList
    // of {text, isHex, crc} maps straight off the edit dialog's ListModel
    // (see AppController::addBatchCommand/updateBatchCommand, which forward
    // here). Ignores an empty (post-trim) name, or a step list that ends up
    // empty once steps with blank (post-trim) text are dropped -- same
    // "ignore rather than store a useless row" reasoning as
    // CommandListModel::addCustomTemplate. update/remove additionally ignore
    // an out-of-range row.
    Q_INVOKABLE void addBatchCommand(const QString &name, int intervalMs, const QVariantList &steps);
    Q_INVOKABLE void updateBatchCommand(int row, const QString &name, int intervalMs, const QVariantList &steps);
    Q_INVOKABLE void removeBatchCommand(int row);

    // The edit dialog's "Edit" flow needs the full per-step isHex/crc back
    // out, not just the roles above -- {text, isHex, crc} maps, same shape
    // addBatchCommand/updateBatchCommand take.
    Q_INVOKABLE QVariantList stepsForRow(int row) const;

    // Not QML-invokable -- AppController reads the whole struct (including
    // per-step isHex/crcEnabled, which QML never needs directly) to actually
    // run a batch.
    const BatchCommand *commandAt(int row) const;

private:
    static QVector<BatchCommandStep> stepsFromVariant(const QVariantList &steps);

    SettingsStore *settings_;
    QVector<BatchCommand> rows_;
};
