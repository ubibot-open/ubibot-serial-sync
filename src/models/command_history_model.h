#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QQmlEngine>
#include <QVector>

class SettingsStore;

// Recently-sent manual text (the bottom "Type data to send…" box), newest
// first. Backs the history dropdown next to the Send button: double-clicking
// an entry there copies it back into the input box.
//
// Persisted through SettingsStore so history survives a restart. Loaded once
// at construction, re-saved after every push()/clear().
class CommandHistoryModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by AppController")

public:
    enum Roles { TextRole = Qt::UserRole + 1, TimeRole };

    // Oldest entries beyond this are dropped on push() -- generous enough to
    // be useful, small enough that the persisted settings blob stays tiny.
    static constexpr int kMaxEntries = 50;

    explicit CommandHistoryModel(SettingsStore *settings, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Adds text as the newest entry. A no-op if it's already the newest
    // entry (as opposed to merely present somewhere in the list) -- repeat
    // send fires this on every tick with the same text, and without this
    // check it would otherwise flood the list with one row per tick. Any
    // older occurrence of the same text is removed so the list has no
    // duplicates.
    Q_INVOKABLE void push(const QString &text);
    Q_INVOKABLE QString textAt(int row) const;
    Q_INVOKABLE void clear();

private:
    void save();

    struct Entry {
        QString text;
        QDateTime sentAt;
    };

    SettingsStore *settings_;
    QVector<Entry> entries_;
};
