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
    enum Roles {
        GroupRole = Qt::UserRole + 1,
        NameRole,
        CmdRole,
        HasParamsRole,
        FavoriteRole,
        IsCustomRole,
    };

    // filterKey sentinels selecting the favorites-only / custom-templates-
    // only view. Any other non-empty value is a literal group name
    // (LocalizedText::zh); empty means "all" -- which, unlike every other
    // non-empty filterKey, still includes the custom templates (see
    // rebuild()).
    static const QString kFavoritesFilterKey;
    static const QString kCustomTemplatesFilterKey;

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

    // "My templates" CRUD -- see SettingsStore::customTemplates(). Adding
    // ignores an empty (post-trim) name or content rather than storing a
    // useless row; update/remove ignore an out-of-range row or one that
    // isn't actually a custom template (defensive -- CommandLibraryPanel.qml
    // only ever offers edit/delete on rows with isCustom true).
    Q_INVOKABLE void addCustomTemplate(const QString &name, const QString &content);
    Q_INVOKABLE void updateCustomTemplate(int row, const QString &name, const QString &content);
    Q_INVOKABLE void removeCustomTemplate(int row);

    // Not QML-invokable -- for AppController (same C++ binary) to reach the
    // actual DeviceCommand behind a row when sending/resolving parameters.
    const DeviceCommand *commandAt(int row) const;

signals:
    void filterChanged();
    void chipsChanged();

private:
    void rebuild();
    void rebuildChips();
    void reloadCustomTemplates();
    bool isFavorite(const DeviceCommand &cmd) const;

    DeviceLibrary *library_;
    SettingsStore *settings_;
    QString modelId_;
    QString search_;
    QString filterKey_;
    QVector<DeviceCommand> rows_;
    // The user's "My templates" list, converted to DeviceCommand once here
    // (isCustom: true, cmdTemplate: the template's content) and re-merged
    // into rows_ on every rebuild() regardless of modelId_ -- reloaded from
    // SettingsStore whenever add/update/removeCustomTemplate changes it.
    QVector<DeviceCommand> customRows_;
    QVariantList chips_;
};
