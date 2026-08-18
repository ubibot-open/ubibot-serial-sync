#include "models/command_list_model.h"
#include "core/language_manager.h"
#include "core/settings_store.h"

#include <QHash>
#include <QUuid>
#include <algorithm>

CommandListModel::CommandListModel(DeviceLibrary *library, SettingsStore *settings, QObject *parent)
    : QAbstractListModel(parent), library_(library), settings_(settings) {
    reloadCustomTemplates();
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, [this] {
        // Name text is localized; re-render existing rows in place rather
        // than reshuffling which commands are shown.
        if (!rows_.isEmpty()) emit dataChanged(index(0), index(rows_.size() - 1), {NameRole});
    });
}

void CommandListModel::setModelId(const QString &id) {
    if (modelId_ == id) return;
    modelId_ = id;
    search_.clear();
    rebuild();
    emit searchChanged();
}

int CommandListModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : rows_.size();
}

QVariant CommandListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rows_.size()) return {};
    const DeviceCommand &cmd = rows_.at(index.row());
    switch (role) {
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
    case IsCustomRole: return cmd.isCustom;
    }
    return {};
}

QHash<int, QByteArray> CommandListModel::roleNames() const {
    return {
        {NameRole, "name"},
        {CmdRole, "cmd"},
        {HasParamsRole, "hasParams"},
        {IsCustomRole, "isCustom"},
    };
}

void CommandListModel::setSearchText(const QString &text) {
    if (search_ == text) return;
    search_ = text;
    rebuild();
    emit searchChanged();
}

namespace {
// Commands added under the new JSON-protocol schema (and custom templates,
// whose id is always a QUuid -- see addCustomTemplate()) carry a stable
// `id`; legacy AT commands don't, so those keep using name.zh like
// favorites used to (and still lose their place in the manual order if
// renamed -- same pre-existing caveat that key had). See
// docs/device-json-protocol-schema.md#11.
QString commandKey(const DeviceCommand &cmd) { return cmd.id.isEmpty() ? cmd.name.zh : cmd.id; }
}  // namespace

void CommandListModel::moveRow(int from, int to) {
    if (from < 0 || from >= rows_.size() || to < 0 || to >= rows_.size() || from == to) return;

    // QAbstractItemModel's own move-rows convention: the destination is
    // expressed as an insert-before position in the *pre-move* index space,
    // which is `to + 1` when moving forward (the item currently at `to`
    // hasn't shifted left yet from this call's point of view) and just `to`
    // when moving backward.
    const int destinationRow = to > from ? to + 1 : to;
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), destinationRow);
    rows_.move(from, to);
    endMoveRows();

    // Persist the *complete* resulting order (every row, not just the two
    // that swapped) so a future rebuild() -- a search, a model switch, or
    // just relaunching the app -- reproduces it instead of reverting to
    // the default. See rebuild() for how a row this list has never seen
    // (no entry here at all) still gets a sensible fallback position.
    QStringList order;
    order.reserve(rows_.size());
    for (const DeviceCommand &cmd : rows_) order.push_back(commandKey(cmd));
    settings_->setCommandOrder(order);
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

    // Default order: custom templates first, then the current model's
    // bundled commands -- no more group/favorites filtering, just matched
    // against the search box. moveRow()'s saved order (below) overrides
    // this for anything it has an explicit position for.
    for (const DeviceCommand &cmd : customRows_) {
        if (matchesSearch(cmd, q)) rows_.push_back(cmd);
    }
    if (const DeviceModel *model = library_->model(modelId_)) {
        for (const DeviceCommand &cmd : model->commands) {
            if (matchesSearch(cmd, q)) rows_.push_back(cmd);
        }
    }

    // Apply the user's own manual order, if they've ever dragged a row --
    // stable_sort so anything with no saved position (a different model's
    // command the drag never touched, say) just keeps the default relative
    // position it was pushed in above, sorting after everything that does
    // have one.
    const QStringList order = settings_->commandOrder();
    if (!order.isEmpty()) {
        QHash<QString, int> rank;
        rank.reserve(order.size());
        for (int i = 0; i < order.size(); ++i) rank.insert(order.at(i), i);
        std::stable_sort(rows_.begin(), rows_.end(), [&](const DeviceCommand &a, const DeviceCommand &b) {
            const auto ra = rank.constFind(commandKey(a));
            const auto rb = rank.constFind(commandKey(b));
            const bool foundA = ra != rank.constEnd();
            const bool foundB = rb != rank.constEnd();
            if (foundA && foundB) return ra.value() < rb.value();
            return foundA && !foundB;
        });
    }

    endResetModel();
}
