#pragma once

#include "core/device_library.h"

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QVariantList>

class SettingsStore;

// Flat, searched view over one device model's command list plus the user's
// own "My templates" -- no grouping, no favorites/chips (removed per user
// feedback that a handful of commands per model didn't need organizing).
// Default order is custom templates first, then the bundled devices.json
// commands; the user can drag any row to reorder the whole list by hand
// (CommandLibraryPanel.qml's delegate calls moveRow() below), which
// persists via SettingsStore::commandOrder() and takes over from the
// default from then on. An earlier revision auto-sorted by most-recent-use
// instead -- removed per user feedback that it made the list feel
// unpredictable.
class CommandListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by AppController")

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        CmdRole,
        HasParamsRole,
        IsCustomRole,
    };

    CommandListModel(DeviceLibrary *library, SettingsStore *settings, QObject *parent = nullptr);

    void setModelId(const QString &id);
    QString modelId() const { return modelId_; }

    // Rebuilds rows_ from the current modelId_ against whatever `library_`
    // now holds -- setModelId() skips rebuild() when the id is unchanged, so
    // this is the hook AppController calls after swapping the library's
    // contents wholesale (DeviceLibrary::loadFromJsonText(), applying a
    // downloaded update) to pick up the new command set for the model
    // that's already selected.
    void reload() { rebuild(); }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString searchText() const { return search_; }
    void setSearchText(const QString &text);

    // Moves the row at `from` to `to` (drag-and-drop reorder -- see
    // CommandLibraryPanel.qml's delegate) and persists the resulting
    // *complete* order via SettingsStore::setCommandOrder(). Uses
    // beginMoveRows()/endMoveRows() rather than a full model reset
    // specifically so the dragged delegate's own Item identity (and the
    // MouseArea grab driving the drag) survives every intermediate swap,
    // not just the final drop.
    Q_INVOKABLE void moveRow(int from, int to);

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
    void searchChanged();

private:
    void rebuild();
    void reloadCustomTemplates();

    DeviceLibrary *library_;
    SettingsStore *settings_;
    QString modelId_;
    QString search_;
    QVector<DeviceCommand> rows_;
    // The user's "My templates" list, converted to DeviceCommand once here
    // (isCustom: true, cmdTemplate: the template's content) and re-merged
    // into rows_ on every rebuild() regardless of modelId_ -- reloaded from
    // SettingsStore whenever add/update/removeCustomTemplate changes it.
    QVector<DeviceCommand> customRows_;
};
