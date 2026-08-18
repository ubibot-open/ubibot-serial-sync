#include "models/command_list_model.h"
#include "core/language_manager.h"
#include "core/settings_store.h"

#include <QUuid>

const QString CommandListModel::kFavoritesFilterKey = QStringLiteral("__favorites__");
const QString CommandListModel::kCustomTemplatesFilterKey = QStringLiteral("__customTemplates__");

CommandListModel::CommandListModel(DeviceLibrary *library, SettingsStore *settings, QObject *parent)
    : QAbstractListModel(parent), library_(library), settings_(settings) {
    reloadCustomTemplates();
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
    case IsCustomRole: return cmd.isCustom;
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
        {IsCustomRole, "isCustom"},
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
    // Custom templates aren't favoritable -- CommandLibraryPanel.qml shows
    // edit/delete buttons instead of a favorite star for these rows, so
    // this shouldn't normally be reachable, but guard anyway rather than
    // writing a meaningless favorites-setting entry keyed by a template id.
    if (cmd.isCustom) return;
    const bool fav = !isFavorite(cmd);
    settings_->setFavorite(modelId_, favoriteKey(cmd), fav);
    emit dataChanged(index(row), index(row), {FavoriteRole});
    if (filterKey_ == kFavoritesFilterKey && !fav) rebuild();
}

void CommandListModel::addCustomTemplate(const QString &name, const QString &content) {
    const QString trimmedName = name.trimmed();
    const QString trimmedContent = content.trimmed();
    if (trimmedName.isEmpty() || trimmedContent.isEmpty()) return;

    QVector<CustomCommandTemplate> templates = settings_->customTemplates();
    CustomCommandTemplate t;
    t.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    t.name = trimmedName;
    t.content = trimmedContent;
    templates.push_back(t);
    settings_->setCustomTemplates(templates);

    reloadCustomTemplates();
    rebuild();
}

void CommandListModel::updateCustomTemplate(int row, const QString &name, const QString &content) {
    if (row < 0 || row >= rows_.size() || !rows_.at(row).isCustom) return;
    const QString trimmedName = name.trimmed();
    const QString trimmedContent = content.trimmed();
    if (trimmedName.isEmpty() || trimmedContent.isEmpty()) return;

    const QString id = rows_.at(row).id;
    QVector<CustomCommandTemplate> templates = settings_->customTemplates();
    for (CustomCommandTemplate &t : templates) {
        if (t.id != id) continue;
        t.name = trimmedName;
        t.content = trimmedContent;
        break;
    }
    settings_->setCustomTemplates(templates);

    reloadCustomTemplates();
    rebuild();
}

void CommandListModel::removeCustomTemplate(int row) {
    if (row < 0 || row >= rows_.size() || !rows_.at(row).isCustom) return;

    const QString id = rows_.at(row).id;
    QVector<CustomCommandTemplate> templates = settings_->customTemplates();
    for (int i = 0; i < templates.size(); ++i) {
        if (templates.at(i).id != id) continue;
        templates.removeAt(i);
        break;
    }
    settings_->setCustomTemplates(templates);

    reloadCustomTemplates();
    rebuild();
}

const DeviceCommand *CommandListModel::commandAt(int row) const {
    if (row < 0 || row >= rows_.size()) return nullptr;
    return &rows_.at(row);
}

void CommandListModel::reloadCustomTemplates() {
    customRows_.clear();
    for (const CustomCommandTemplate &t : settings_->customTemplates()) {
        DeviceCommand cmd;
        cmd.id = t.id;
        // Plain user-authored text, not run through LocalizedText's zh/en
        // pick -- setting both to the same string is what makes it show up
        // identically regardless of interface language.
        cmd.name.zh = t.name;
        cmd.name.en = t.name;
        // "My templates" section header -- an app-owned label, so (unlike
        // the row's own name/content above) this *does* get an actual zh/en
        // pair, same convention devices.json's own group labels use.
        cmd.group.zh = QStringLiteral("我的模板");
        cmd.group.en = QStringLiteral("My templates");
        cmd.cmdTemplate = t.content;
        cmd.isCustom = true;
        customRows_.push_back(cmd);
    }
}

namespace {
bool matchesSearch(const DeviceCommand &cmd, const QString &q) {
    if (q.isEmpty()) return true;
    return cmd.name.zh.toLower().contains(q) || cmd.name.en.toLower().contains(q) ||
           cmd.cmdTemplate.toLower().contains(q);
}
}  // namespace

void CommandListModel::rebuild() {
    beginResetModel();
    rows_.clear();
    const QString q = search_.trimmed().toLower();

    // "My templates" filter shows only the custom rows below and skips the
    // current model's own commands entirely; every other filterKey (a
    // specific group, favorites, or "all") only ever matches bundled
    // devices.json commands, so the custom rows stay out of those views.
    if (filterKey_ != kCustomTemplatesFilterKey) {
        if (const DeviceModel *model = library_->model(modelId_)) {
            for (const DeviceCommand &cmd : model->commands) {
                if (!matchesSearch(cmd, q)) continue;
                if (filterKey_ == kFavoritesFilterKey) {
                    if (!isFavorite(cmd)) continue;
                } else if (!filterKey_.isEmpty() && cmd.group.zh != filterKey_) {
                    continue;
                }
                rows_.push_back(cmd);
            }
        }
    }

    // Custom templates show up in "all" (empty filterKey) and in their own
    // dedicated filter, regardless of which device model is selected --
    // they aren't associated with one.
    if (filterKey_.isEmpty() || filterKey_ == kCustomTemplatesFilterKey) {
        for (const DeviceCommand &cmd : customRows_) {
            if (!matchesSearch(cmd, q)) continue;
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
    addChip(tr("My templates"), kCustomTemplatesFilterKey);

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
