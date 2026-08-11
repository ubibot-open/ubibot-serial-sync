#include "ui/command_library_panel.h"
#include "core/settings_store.h"
#include "ui/flow_layout.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace {

// One row in the command tree: a star (favorite) toggle plus the command's
// bilingual name and literal AT-command text. Clicking anywhere except the
// star activates the command; the star toggles favorite status independently.
class CommandRowWidget : public QWidget {
    Q_OBJECT
public:
    explicit CommandRowWidget(const DeviceCommand &cmd, bool favorite, QWidget *parent = nullptr)
        : QWidget(parent), cmd_(cmd) {
        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(6, 4, 6, 4);
        layout->setSpacing(9);

        star_ = new QToolButton;
        star_->setCheckable(true);
        star_->setChecked(favorite);
        star_->setAutoRaise(true);
        star_->setText(QStringLiteral("★"));  // ★
        star_->setStyleSheet(starStyle(favorite));
        connect(star_, &QToolButton::toggled, this, [this](bool on) {
            star_->setStyleSheet(starStyle(on));
            emit favoriteToggled(cmd_, on);
        });
        layout->addWidget(star_);

        auto *textCol = new QVBoxLayout;
        textCol->setSpacing(1);
        nameLabel_ = new QLabel(cmd_.name.text());
        QFont nameFont = nameLabel_->font();
        nameFont.setBold(true);
        nameLabel_->setFont(nameFont);
        cmdLabel_ = new QLabel(cmd_.cmdTemplate);
        cmdLabel_->setStyleSheet(QStringLiteral("color:#416180; font-family: Consolas, monospace; font-size: 11px;"));
        textCol->addWidget(nameLabel_);
        textCol->addWidget(cmdLabel_);
        layout->addLayout(textCol, 1);
    }

    void refreshText() { nameLabel_->setText(cmd_.name.text()); }

signals:
    void activated(const DeviceCommand &cmd);
    void favoriteToggled(const DeviceCommand &cmd, bool fav);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (!star_->geometry().contains(event->pos())) emit activated(cmd_);
        QWidget::mousePressEvent(event);
    }

private:
    static QString starStyle(bool on) {
        return QStringLiteral("QToolButton { border: none; color: %1; font-size: 14px; }")
            .arg(on ? QStringLiteral("#5980a6") : QStringLiteral("#b7b7ba"));
    }

    DeviceCommand cmd_;
    QToolButton *star_;
    QLabel *nameLabel_;
    QLabel *cmdLabel_;
};

}  // namespace

CommandLibraryPanel::CommandLibraryPanel(DeviceLibrary *library, SettingsStore *settings, QWidget *parent)
    : QWidget(parent), library_(library), settings_(settings) {
    buildUi();
    retranslateUi();  // also performs the initial chip/tree population
}

void CommandLibraryPanel::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(10, 10, 10, 0);
    root->setSpacing(10);

    modelLabel_ = new QLabel;
    modelCombo_ = new QComboBox;
    for (const QString &id : library_->modelIds()) modelCombo_->addItem(id);
    connect(modelCombo_, &QComboBox::currentTextChanged, this, [this](const QString &id) {
        filterKind_ = FilterKind::All;
        const DeviceModel *model = library_->model(id);
        modelDescLabel_->setText(model ? model->description.text() : QString());
        rebuildFilterChips();
        rebuildCommandTree();
        emit modelChanged(id);
    });

    modelDescLabel_ = new QLabel;
    modelDescLabel_->setWordWrap(true);
    modelDescLabel_->setStyleSheet(QStringLiteral("color: #5d5d60; font-size: 12px;"));

    searchEdit_ = new QLineEdit;
    connect(searchEdit_, &QLineEdit::textChanged, this, [this] { rebuildCommandTree(); });

    chipsContainer_ = new QWidget;
    chipsLayout_ = new FlowLayout(chipsContainer_, 0, 6, 6);

    tree_ = new QTreeWidget;
    tree_->setHeaderHidden(true);
    tree_->setIndentation(0);
    tree_->setRootIsDecorated(false);
    tree_->setFrameShape(QFrame::NoFrame);
    tree_->setSelectionMode(QAbstractItemView::NoSelection);

    root->addWidget(modelLabel_);
    root->addWidget(modelCombo_);
    root->addWidget(modelDescLabel_);
    root->addWidget(searchEdit_);
    root->addWidget(chipsContainer_);
    root->addWidget(tree_, 1);
}

QString CommandLibraryPanel::currentModelId() const { return modelCombo_->currentText(); }

void CommandLibraryPanel::setCurrentModelId(const QString &id) {
    const int idx = modelCombo_->findText(id);
    if (idx >= 0) modelCombo_->setCurrentIndex(idx);
}

void CommandLibraryPanel::rebuildFilterChips() {
    QLayoutItem *item;
    while ((item = chipsLayout_->takeAt(0))) {
        delete item->widget();
        delete item;
    }

    auto addChip = [this](const QString &label, FilterKind kind, const QString &groupKey) {
        auto *btn = new QPushButton(label);
        btn->setCheckable(true);
        btn->setFlat(false);
        const bool checked = (filterKind_ == kind) && (kind != FilterKind::Group || groupKey == filterGroupKey_);
        btn->setChecked(checked);
        connect(btn, &QPushButton::clicked, this, [this, kind, groupKey] {
            filterKind_ = kind;
            filterGroupKey_ = groupKey;
            rebuildFilterChips();
            rebuildCommandTree();
        });
        chipsLayout_->addWidget(btn);
    };

    addChip(tr("All"), FilterKind::All, QString());
    addChip(tr("Favorites"), FilterKind::Favorites, QString());

    const DeviceModel *model = library_->model(currentModelId());
    if (model) {
        QStringList seen;
        for (const DeviceCommand &cmd : model->commands) {
            if (seen.contains(cmd.group.zh)) continue;
            seen.push_back(cmd.group.zh);
            addChip(cmd.group.text(), FilterKind::Group, cmd.group.zh);
        }
    }
}

bool CommandLibraryPanel::isFavorite(const DeviceCommand &cmd) const {
    return settings_->isFavorite(currentModelId(), cmd.name.zh);
}

void CommandLibraryPanel::toggleFavorite(const DeviceCommand &cmd, bool fav) {
    settings_->setFavorite(currentModelId(), cmd.name.zh, fav);
    if (filterKind_ == FilterKind::Favorites) rebuildCommandTree();
}

QVector<DeviceCommand> CommandLibraryPanel::matchingCommands() const {
    QVector<DeviceCommand> result;
    const DeviceModel *model = library_->model(currentModelId());
    if (!model) return result;

    const QString q = searchEdit_->text().trimmed().toLower();
    for (const DeviceCommand &cmd : model->commands) {
        if (!q.isEmpty()) {
            const bool nameMatch = cmd.name.zh.toLower().contains(q) || cmd.name.en.toLower().contains(q);
            const bool cmdMatch = cmd.cmdTemplate.toLower().contains(q);
            if (!nameMatch && !cmdMatch) continue;
        }
        if (filterKind_ == FilterKind::Favorites && !isFavorite(cmd)) continue;
        if (filterKind_ == FilterKind::Group && cmd.group.zh != filterGroupKey_) continue;
        result.push_back(cmd);
    }
    return result;
}

void CommandLibraryPanel::rebuildCommandTree() {
    tree_->clear();

    const QVector<DeviceCommand> matches = matchingCommands();
    QHash<QString, QTreeWidgetItem *> groupItems;

    for (const DeviceCommand &cmd : matches) {
        if (!groupItems.contains(cmd.group.zh)) {
            auto *groupItem = new QTreeWidgetItem(tree_);
            groupItem->setFlags(Qt::ItemIsEnabled);
            groupItem->setFirstColumnSpanned(true);
            auto *label = new QLabel(cmd.group.text().toUpper());
            label->setStyleSheet(
                QStringLiteral("color:#416180; font-size: 10px; letter-spacing: 1px; padding: 4px 4px;"));
            tree_->setItemWidget(groupItem, 0, label);
            groupItems.insert(cmd.group.zh, groupItem);
        }

        auto *item = new QTreeWidgetItem(groupItems[cmd.group.zh]);
        auto *row = new CommandRowWidget(cmd, isFavorite(cmd));
        connect(row, &CommandRowWidget::activated, this, [this](const DeviceCommand &c) {
            if (c.hasParams())
                emit commandParamsRequested(c);
            else
                emit commandActivated(c);
        });
        connect(row, &CommandRowWidget::favoriteToggled, this,
                [this](const DeviceCommand &c, bool fav) { toggleFavorite(c, fav); });
        tree_->setItemWidget(item, 0, row);
    }

    tree_->expandAll();
}

void CommandLibraryPanel::retranslateUi() {
    modelLabel_->setText(tr("Device model"));
    const DeviceModel *model = library_->model(currentModelId());
    modelDescLabel_->setText(model ? model->description.text() : QString());
    searchEdit_->setPlaceholderText(tr("Search commands"));
    rebuildFilterChips();
    rebuildCommandTree();
}

void CommandLibraryPanel::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}

#include "command_library_panel.moc"
