#pragma once

#include "core/device_library.h"

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QVariantList>

class SettingsStore;

// Flat, searched view over one device model's command list plus the user's
// own "My templates" -- no grouping, no favorites/chips (removed per user
// feedback that a handful of commands per model didn't need organizing);
// sorted by CommandListModel::rebuild() so whichever row was clicked most
// recently (across either source) always sorts first, back to a QML
// ListView with no section headers of its own.
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

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString searchText() const { return search_; }
    void setSearchText(const QString &text);

    // Records "now" as this row's last-used time (see
    // SettingsStore::recordCommandUsed()) and re-sorts so it moves to the
    // top -- called once per row click from AppController::
    // loadCommandIntoDraft()/loadCommandWithParamsIntoDraft(), whichever
    // source the row came from.
    Q_INVOKABLE void recordUsed(int row);

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
