#include "models/command_list_model.h"
#include "core/language_manager.h"
#include "core/settings_store.h"

const QString CommandListModel::kFavoritesFilterKey = QStringLiteral("__favorites__");

CommandListModel::CommandListModel(DeviceLibrary *library, SettingsStore *settings, QObject *parent)
    : QAbstractListModel(parent), library_(library), settings_(settings) {
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, [this] {
        rebuildChips();
        // Group/name text is localized; re-render existing rows in place
        // rather than reshuffling which commands are shown.
        if (!rows_.isEmpty()) emit dataChanged(index(0), index(rows_.size() - 1), {GroupRole, NameRole});
    });
}

void CommandListModel::setModelId(const QString &id) {
    if (modelId_ == id) return;
    modelId_ = id;
    filterKey_.clear();
    search_.clear();
    rebuild();
    rebuildChips();
    emit filterChanged();
}

int CommandListModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : rows_.size();
}

QVariant CommandListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rows_.size()) return {};
    const DeviceCommand &cmd = rows_.at(index.row());
    switch (role) {
    case GroupRole: return cmd.group.text();
    case NameRole: return cmd.name.text();
    case CmdRole: return cmd.wirePayload();
    // AT protocol only: JSON commands' own `params` is documentation-only
    // (see docs/device-json-protocol-schema.md#6) -- it lists what
    // placeholders the payload carries, but was never meant to open the
    // AT-style structured params form, whose DeviceCommand::resolve() only
    // substitutes into cmdTemplate (empty for JSON commands, which stage
    // their raw wirePayload() -- placeholders and all -- into the
    // manual-send box instead). Gating on isJsonProtocol here is what keeps
    // CommandLibraryPanel.qml's row click routing correct for a JSON
    // command that happens to carry a non-empty params array.
    case HasParamsRole: return !cmd.isJsonProtocol && cmd.hasParams();
    case FavoriteRole: return isFavorite(cmd);
    }
    return {};
}

QHash<int, QByteArray> CommandListModel::roleNames() const {
    return {
        {GroupRole, "group"},
        {NameRole, "name"},
        {CmdRole, "cmd"},
        {HasParamsRole, "hasParams"},
        {FavoriteRole, "favorite"},
    };
}

void CommandListModel::setSearchText(const QString &text) {
    if (search_ == text) return;
    search_ = text;
    rebuild();
    emit filterChanged();
}

void CommandListModel::setFilterKey(const QString &key) {
    if (filterKey_ == key) return;
    filterKey_ = key;
    rebuild();
    rebuildChips();
    emit filterChanged();
}

namespace {
// Commands added under the new JSON-protocol schema carry a stable `id`;
// legacy AT commands don't, so those keep using name.zh like before (and
// still lose their favorite if renamed -- pre-existing behavior, unchanged
// here). See docs/device-json-protocol-schema.md#11.
QString favoriteKey(const DeviceCommand &cmd) { return cmd.id.isEmpty() ? cmd.name.zh : cmd.id; }
}  // namespace

bool CommandListModel::isFavorite(const DeviceCommand &cmd) const {
    return settings_->isFavorite(modelId_, favoriteKey(cmd));
}

void CommandListModel::toggleFavorite(int row) {
    if (row < 0 || row >= rows_.size()) return;
    const DeviceCommand &cmd = rows_.at(row);
    const bool fav = !isFavorite(cmd);
    settings_->setFavorite(modelId_, favoriteKey(cmd), fav);
    emit dataChanged(index(row), index(row), {FavoriteRole});
    if (filterKey_ == kFavoritesFilterKey && !fav) rebuild();
}

const DeviceCommand *CommandListModel::commandAt(int row) const {
    if (row < 0 || row >= rows_.size()) return nullptr;
    return &rows_.at(row);
}

void CommandListModel::rebuild() {
    beginResetModel();
    rows_.clear();
    if (const DeviceModel *model = library_->model(modelId_)) {
        const QString q = search_.trimmed().toLower();
        for (const DeviceCommand &cmd : model->commands) {
            if (!q.isEmpty()) {
                const bool nameMatch = cmd.name.zh.toLower().contains(q) || cmd.name.en.toLower().contains(q);
                const bool cmdMatch = cmd.cmdTemplate.toLower().contains(q);
                if (!nameMatch && !cmdMatch) continue;
            }
            if (filterKey_ == kFavoritesFilterKey) {
                if (!isFavorite(cmd)) continue;
            } else if (!filterKey_.isEmpty() && cmd.group.zh != filterKey_) {
                continue;
            }
            rows_.push_back(cmd);
        }
    }
    endResetModel();
}

void CommandListModel::rebuildChips() {
    QVariantList chips;
    auto addChip = [&](const QString &label, const QString &key) {
        QVariantMap m;
        m[QStringLiteral("label")] = label;
        m[QStringLiteral("key")] = key;
        m[QStringLiteral("checked")] = (filterKey_ == key);
        chips.push_back(m);
    };
    addChip(tr("All"), QString());
    addChip(tr("Favorites"), kFavoritesFilterKey);

    if (const DeviceModel *model = library_->model(modelId_)) {
        QStringList seen;
        for (const DeviceCommand &cmd : model->commands) {
            if (seen.contains(cmd.group.zh)) continue;
            seen.push_back(cmd.group.zh);
            addChip(cmd.group.text(), cmd.group.zh);
        }
    }
    chips_ = chips;
    emit chipsChanged();
}
