#pragma once

#include "core/device_library.h"

#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class FlowLayout;
class SettingsStore;

// Left-hand panel shown in "Device commands" mode: model picker, search,
// favorite/group filter chips, and the grouped command list itself. This is
// the panel that turns the app from a generic serial terminal into a
// UbiBot-aware tool.
class CommandLibraryPanel : public QWidget {
    Q_OBJECT
public:
    CommandLibraryPanel(DeviceLibrary *library, SettingsStore *settings, QWidget *parent = nullptr);

    QString currentModelId() const;
    void setCurrentModelId(const QString &id);

signals:
    // Emitted when a parameter-less command row is clicked: ready to send as-is.
    void commandActivated(const DeviceCommand &cmd);
    // Emitted when a command that needs parameters is clicked: MainWindow
    // should surface CommandParamsPanel for it.
    void commandParamsRequested(const DeviceCommand &cmd);
    void modelChanged(const QString &modelId);

protected:
    void changeEvent(QEvent *event) override;

private:
    enum class FilterKind { All, Favorites, Group };

    void buildUi();
    void retranslateUi();
    void rebuildFilterChips();
    void rebuildCommandTree();
    QVector<DeviceCommand> matchingCommands() const;
    void toggleFavorite(const DeviceCommand &cmd, bool fav);
    bool isFavorite(const DeviceCommand &cmd) const;

    DeviceLibrary *library_;
    SettingsStore *settings_;

    QComboBox *modelCombo_;
    QLabel *modelLabel_;
    QLabel *modelDescLabel_;
    QLineEdit *searchEdit_;
    QWidget *chipsContainer_;
    FlowLayout *chipsLayout_;
    QTreeWidget *tree_;

    FilterKind filterKind_ = FilterKind::All;
    QString filterGroupKey_;  // group.zh, used as a stable key across languages
};
