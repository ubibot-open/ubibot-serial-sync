#include "models/command_history_model.h"
#include "core/settings_store.h"

CommandHistoryModel::CommandHistoryModel(SettingsStore *settings, QObject *parent)
    : QAbstractListModel(parent), settings_(settings) {
    // Persisted history is text-only (see SettingsStore::commandHistory) --
    // sentAt stays invalid for these, and TimeRole below just returns an
    // empty string for them rather than a fabricated time.
    for (const QString &text : settings_->commandHistory()) entries_.push_back({text, QDateTime()});
}

int CommandHistoryModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : entries_.size();
}

QVariant CommandHistoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) return {};
    const Entry &e = entries_.at(index.row());
    switch (role) {
    case TextRole: return e.text;
    case TimeRole: return e.sentAt.isValid() ? e.sentAt.toString(QStringLiteral("HH:mm:ss")) : QString();
    }
    return {};
}

QHash<int, QByteArray> CommandHistoryModel::roleNames() const {
    // Exposed as "commandText" rather than "text" -- ItemDelegate already
    // has a built-in "text" property (its button label), and a delegate
    // redeclaring a role-backed "required property string text" over that
    // would be a duplicate-property QML error.
    return {
        {TextRole, "commandText"},
        {TimeRole, "timeText"},
    };
}

void CommandHistoryModel::push(const QString &text) {
    if (text.isEmpty()) return;
    if (!entries_.isEmpty() && entries_.first().text == text) return;

    beginResetModel();
    // Drop any older occurrence of the same text so it doesn't appear twice
    // once re-inserted at the front.
    for (int i = entries_.size() - 1; i >= 0; --i) {
        if (entries_.at(i).text == text) entries_.remove(i);
    }
    entries_.prepend({text, QDateTime::currentDateTime()});
    while (entries_.size() > kMaxEntries) entries_.removeLast();
    endResetModel();

    save();
}

QString CommandHistoryModel::textAt(int row) const {
    return (row >= 0 && row < entries_.size()) ? entries_.at(row).text : QString();
}

void CommandHistoryModel::clear() {
    if (entries_.isEmpty()) return;
    beginResetModel();
    entries_.clear();
    endResetModel();
    save();
}

void CommandHistoryModel::save() {
    QStringList texts;
    texts.reserve(entries_.size());
    for (const Entry &e : entries_) texts.push_back(e.text);
    settings_->setCommandHistory(texts);
}
