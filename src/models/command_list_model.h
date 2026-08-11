#pragma once

#include "core/device_library.h"

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QVariantList>

class SettingsStore;

// Flat, searched/filtered view over one device model's command list, meant
// to back a QML ListView whose delegate groups consecutive rows by the
// "group" role via ListView's built-in `section.property` -- no separate
// header rows to manage by hand, unlike the old QTreeWidget version.
class CommandListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by AppController")

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY filterChanged)
    Q_PROPERTY(QString filterKey READ filterKey WRITE setFilterKey NOTIFY filterChanged)
    Q_PROPERTY(QVariantList filterChips READ filterChips NOTIFY chipsChanged)

public:
    enum Roles { GroupRole = Qt::UserRole + 1, NameRole, CmdRole, HasParamsRole, FavoriteRole };

    // filterKey sentinel selecting the favorites-only view. Any other
    // non-empty value is a literal group name (LocalizedText::zh); empty
    // means "all".
    static const QString kFavoritesFilterKey;

    CommandListModel(DeviceLibrary *library, SettingsStore *settings, QObject *parent = nullptr);

    void setModelId(const QString &id);
    QString modelId() const { return modelId_; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString searchText() const { return search_; }
    void setSearchText(const QString &text);
    QString filterKey() const { return filterKey_; }
    void setFilterKey(const QString &key);
    QVariantList filterChips() const { return chips_; }

    Q_INVOKABLE void toggleFavorite(int row);

    // Not QML-invokable -- for AppController (same C++ binary) to reach the
    // actual DeviceCommand behind a row when sending/resolving parameters.
    const DeviceCommand *commandAt(int row) const;

signals:
    void filterChanged();
    void chipsChanged();

private:
    void rebuild();
    void rebuildChips();
    bool isFavorite(const DeviceCommand &cmd) const;

    DeviceLibrary *library_;
    SettingsStore *settings_;
    QString modelId_;
    QString search_;
    QString filterKey_;
    QVector<DeviceCommand> rows_;
    QVariantList chips_;
};
