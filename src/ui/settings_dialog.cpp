// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/settings_dialog.h"

#include "app/app_controller.h"
#include "ui/ui_common.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>

#include <QLight/LineEdit.h>
#include <QLight/ProgressBar.h>
#include <QLight/PushButton.h>
#include <QLight/SpinBox.h>

namespace ui {

namespace {

ThemedLabel *fieldLabel(QWidget *parent) {
    auto *l = new ThemedLabel(ThemedLabel::Primary, parent);
    l->setEmphasis(true);
    return l;
}

}  // namespace

// --- SettingsDialog ------------------------------------------------------

SettingsDialog::SettingsDialog(AppController *controller, QWidget *parent)
    : AppDialog(parent), controller_(controller) {
    setDialogWidth(480);
    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(6);

    // language
    languageLabel_ = fieldLabel(content);
    languageCombo_ = new ComboBox(content);
    for (const QVariant &v : controller_->availableLanguages()) {
        const QVariantMap m = v.toMap();
        languageCombo_->addItem(m.value(QStringLiteral("nativeName")).toString(), m.value(QStringLiteral("code")));
    }
    col->addWidget(languageLabel_);
    col->addWidget(languageCombo_);

    // fonts
    const QStringList families = controller_->availableFontFamilies();
    auto fontRow = [&](ThemedLabel *&label, ComboBox *&combo, ThemedLabel *&sizeLabel, QLight::SpinBox *&spin) {
        label = fieldLabel(content);
        combo = new ComboBox(content);
        combo->addItems(families);
        combo->setMaxVisibleItems(16);
        combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        combo->setMinimumContentsLength(12);
        sizeLabel = new ThemedLabel(ThemedLabel::Secondary, content);
        spin = new QLight::SpinBox(content);
        spin->setRange(8, 32);
        spin->setMinimumWidth(84);
        auto *row = new QHBoxLayout;
        row->setSpacing(10);
        row->addWidget(combo, 1);
        row->addWidget(sizeLabel);
        row->addWidget(spin);
        col->addSpacing(8);
        col->addWidget(label);
        col->addLayout(row);
    };
    fontRow(systemFontLabel_, systemFontCombo_, systemSizeLabel_, systemSizeSpin_);
    fontRow(logFontLabel_, logFontCombo_, logSizeLabel_, logSizeSpin_);

    // theme
    themeLabel_ = fieldLabel(content);
    themeControl_ = new SegmentedControl(content);
    themeControl_->setOptions({QString(), QString()});
    themeControl_->setSegmentWidth(96);
    col->addSpacing(8);
    col->addWidget(themeLabel_);
    col->addWidget(themeControl_, 0, Qt::AlignLeft);

    // command library
    libraryLabel_ = fieldLabel(content);
    libraryVersion_ = new QLight::LineEdit(content);
    libraryVersion_->setReadOnly(true);
    libraryVersion_->setEnabled(false);
    libraryVersion_->setFont(monoFont());
    libraryBadge_ = new ThemedLabel(ThemedLabel::Accent, content);
    checkButton_ = new QLight::PushButton(content);
    auto *libraryRow = new QHBoxLayout;
    libraryRow->setSpacing(10);
    libraryRow->addWidget(libraryVersion_, 1);
    libraryRow->addWidget(libraryBadge_);
    libraryRow->addWidget(checkButton_);
    col->addSpacing(8);
    col->addWidget(libraryLabel_);
    col->addLayout(libraryRow);

    col->addSpacing(12);
    col->addWidget(new Divider(Qt::Horizontal, content));
    col->addSpacing(8);

    // info
    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(2);
    versionLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    version_ = new ThemedLabel(ThemedLabel::Primary, content);
    platformLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    platform_ = new ThemedLabel(QStringLiteral("Windows · macOS · Linux"), ThemedLabel::Primary, content);
    supportLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    support_ = new ThemedLabel(QStringLiteral("support@ubibot.com"), ThemedLabel::Primary, content);
    support_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    devicesLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    devices_ = new ThemedLabel(ThemedLabel::Primary, content);
    grid->addWidget(versionLabel_, 0, 0);
    grid->addWidget(platformLabel_, 0, 1);
    grid->addWidget(version_, 1, 0);
    grid->addWidget(platform_, 1, 1);
    grid->setRowMinimumHeight(2, 10);
    grid->addWidget(supportLabel_, 3, 0);
    grid->addWidget(devicesLabel_, 3, 1);
    grid->addWidget(support_, 4, 0);
    grid->addWidget(devices_, 4, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    col->addLayout(grid);

    col->addSpacing(12);
    restoreButton_ = new QLight::PushButton(content);
    col->addWidget(restoreButton_, 0, Qt::AlignLeft);
    setContentWidget(content);

    closeButton_ = addAcceptButton(QString());

    // widgets -> controller
    connect(languageCombo_, &QComboBox::activated, this,
            [this](int i) { controller_->setCurrentLanguage(languageCombo_->itemData(i).toString()); });
    connect(systemFontCombo_, &QComboBox::activated, this,
            [this](int i) { controller_->setSystemFontFamily(systemFontCombo_->itemText(i)); });
    connect(systemSizeSpin_, &QSpinBox::valueChanged, this, [this](int v) {
        if (!syncing_) controller_->setSystemFontSize(v);
    });
    connect(logFontCombo_, &QComboBox::activated, this,
            [this](int i) { controller_->setLogFontFamily(logFontCombo_->itemText(i)); });
    connect(logSizeSpin_, &QSpinBox::valueChanged, this, [this](int v) {
        if (!syncing_) controller_->setLogFontSize(v);
    });
    connect(themeControl_, &SegmentedControl::activated, this,
            [this](int i) { controller_->setThemeMode(i == 1 ? QStringLiteral("dark") : QStringLiteral("light")); });
    connect(checkButton_, &QPushButton::clicked, this, [this] {
        controller_->checkForLibraryUpdate();
        if (!updateDialog_) updateDialog_ = new LibraryUpdateDialog(controller_, parentWidget());
        updateDialog_->open();
    });
    connect(restoreButton_, &QPushButton::clicked, controller_, &AppController::restoreDefaultSettings);

    // controller -> widgets
    for (auto signal : {&AppController::currentLanguageChanged, &AppController::systemFontChanged,
                        &AppController::logFontChanged, &AppController::themeModeChanged}) {
        connect(controller_, signal, this, &SettingsDialog::syncFromController);
    }
    connect(controller_, &AppController::libraryChanged, this, &SettingsDialog::updateLibraryState);
    connect(controller_, &AppController::libraryUpdateStateChanged, this, &SettingsDialog::updateLibraryState);

    retranslateUi();
    syncFromController();
}

void SettingsDialog::retranslateUi() {
    setTitle(tr("Settings & About"));
    languageLabel_->setText(tr("Interface language"));
    systemFontLabel_->setText(tr("System font"));
    systemSizeLabel_->setText(tr("Size"));
    logFontLabel_->setText(tr("Data monitor font"));
    logSizeLabel_->setText(tr("Size"));
    themeLabel_->setText(tr("Theme"));
    themeControl_->setOptionText(0, tr("Light"));
    themeControl_->setOptionText(1, tr("Dark"));
    libraryLabel_->setText(tr("Command library"));
    checkButton_->setText(tr("Check for updates"));
    versionLabel_->setText(tr("Version"));
    platformLabel_->setText(tr("Platform"));
    supportLabel_->setText(tr("Support"));
    devicesLabel_->setText(tr("Devices"));
    restoreButton_->setText(tr("Restore defaults"));
    closeButton_->setText(tr("Close"));
    version_->setText(tr("%1 (Qt %2)").arg(controller_->appVersion(), controller_->qtVersion()));
    updateLibraryState();
}

void SettingsDialog::openDialog() {
    syncFromController();
    open();
}

void SettingsDialog::syncFromController() {
    syncing_ = true;
    languageCombo_->setCurrentIndex(languageCombo_->findData(controller_->currentLanguage()));
    systemFontCombo_->setCurrentIndex(systemFontCombo_->findText(controller_->systemFontFamily()));
    systemSizeSpin_->setValue(controller_->systemFontSize());
    logFontCombo_->setCurrentIndex(logFontCombo_->findText(controller_->logFontFamily()));
    logSizeSpin_->setValue(controller_->logFontSize());
    themeControl_->setCurrentIndex(controller_->themeMode() == QLatin1String("dark") ? 1 : 0);
    syncing_ = false;
}

void SettingsDialog::updateLibraryState() {
    libraryVersion_->setText(controller_->libraryVersion());
    const QString state = controller_->libraryUpdateState();
    if (state == QLatin1String("upToDate")) {
        libraryBadge_->setText(tr("Up to date"));
        libraryBadge_->setRole(ThemedLabel::Success);
        libraryBadge_->show();
    } else if (state == QLatin1String("updateAvailable")) {
        libraryBadge_->setText(tr("Update available"));
        libraryBadge_->setRole(ThemedLabel::Accent);
        libraryBadge_->show();
    } else {
        libraryBadge_->hide();
    }
    checkButton_->setEnabled(state != QLatin1String("checking") && state != QLatin1String("downloading"));
    devices_->setText(tr("%1 models · %2 commands").arg(controller_->modelCount()).arg(controller_->commandCount()));
}

// --- LibraryUpdateDialog -------------------------------------------------

LibraryUpdateDialog::LibraryUpdateDialog(AppController *controller, QWidget *parent)
    : AppDialog(parent), controller_(controller) {
    setDialogWidth(400);
    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(10);
    message_ = new ThemedLabel(ThemedLabel::Primary, content);
    message_->setWordWrap(true);
    busy_ = new QLight::ProgressBar(content);
    busy_->setRange(0, 0);
    busy_->setTextVisible(false);
    col->addWidget(message_);
    col->addWidget(busy_);
    setContentWidget(content);

    downloadButton_ = addActionButton(QString(), true);
    okButton_ = addAcceptButton(QString());
    connect(downloadButton_, &QPushButton::clicked, controller_, &AppController::downloadLibraryUpdate);
    connect(controller_, &AppController::libraryUpdateStateChanged, this, &LibraryUpdateDialog::updateState);
    retranslateUi();
}

void LibraryUpdateDialog::retranslateUi() {
    setTitle(tr("Command library"));
    downloadButton_->setText(tr("Download and apply"));
    okButton_->setText(tr("OK"));
    updateState();
}

void LibraryUpdateDialog::updateState() {
    const QString state = controller_->libraryUpdateState();
    message_->setText(controller_->libraryUpdateMessage());
    busy_->setVisible(state == QLatin1String("checking") || state == QLatin1String("downloading"));
    downloadButton_->setVisible(controller_->libraryUpdateAvailable());
    downloadButton_->setEnabled(state != QLatin1String("downloading"));
    refit();
}

// --- SoftwareUpdateDialog ------------------------------------------------

SoftwareUpdateDialog::SoftwareUpdateDialog(AppController *controller, QWidget *parent)
    : AppDialog(parent), controller_(controller) {
    setDialogWidth(420);
    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(8);
    headline_ = new ThemedLabel(ThemedLabel::Primary, content);
    headline_->setEmphasis(true);
    headline_->setWordWrap(true);
    message_ = new ThemedLabel(ThemedLabel::Secondary, content);
    message_->setWordWrap(true);
    progress_ = new QLight::ProgressBar(content);
    progress_->setRange(0, 1000);
    progress_->setTextVisible(false);
    percent_ = new ThemedLabel(ThemedLabel::Secondary, content);
    col->addWidget(headline_);
    col->addWidget(message_);
    col->addWidget(progress_);
    col->addWidget(percent_);
    setContentWidget(content);

    laterButton_ = addRejectButton(QString());
    cancelButton_ = addActionButton(QString());
    retryButton_ = addActionButton(QString());
    updateButton_ = addActionButton(QString(), true);
    okButton_ = addAcceptButton(QString());
    connect(cancelButton_, &QPushButton::clicked, this, [this] {
        controller_->cancelAppUpdateDownload();
        reject();
    });
    connect(retryButton_, &QPushButton::clicked, controller_, &AppController::checkForAppUpdate);
    connect(updateButton_, &QPushButton::clicked, controller_, &AppController::installAppUpdate);
    connect(controller_, &AppController::appUpdateStateChanged, this, &SoftwareUpdateDialog::updateState);
    connect(controller_, &AppController::appUpdateProgressChanged, this, [this] {
        progress_->setValue(qRound(controller_->appUpdateProgress() * 1000));
        percent_->setText(QStringLiteral("%1%").arg(qRound(controller_->appUpdateProgress() * 100)));
    });
    retranslateUi();
}

void SoftwareUpdateDialog::retranslateUi() {
    setTitle(tr("Software Update"));
    laterButton_->setText(tr("Later"));
    cancelButton_->setText(tr("Cancel"));
    retryButton_->setText(tr("Retry"));
    updateButton_->setText(tr("Update now"));
    okButton_->setText(tr("OK"));
    updateState();
}

void SoftwareUpdateDialog::updateState() {
    const QString state = controller_->appUpdateState();
    const bool available = state == QLatin1String("updateAvailable");
    const bool downloading = state == QLatin1String("downloading");
    const bool finished = state == QLatin1String("upToDate") || state == QLatin1String("error");

    headline_->setVisible(available);
    headline_->setText(
        tr("Version %1 is available (you have %2)").arg(controller_->remoteAppVersion(), controller_->appVersion()));
    message_->setText(controller_->appUpdateMessage());
    message_->setRole(state == QLatin1String("error") ? ThemedLabel::Danger
                      : available                     ? ThemedLabel::Secondary
                                                      : ThemedLabel::Primary);
    progress_->setVisible(downloading || state == QLatin1String("checking") || state == QLatin1String("installing"));
    if (!downloading) progress_->setRange(0, 0);
    else progress_->setRange(0, 1000);
    percent_->setVisible(downloading);

    laterButton_->setVisible(available);
    cancelButton_->setVisible(downloading);
    updateButton_->setVisible(available);
    retryButton_->setVisible(state == QLatin1String("error"));
    okButton_->setVisible(finished);
    refit();
}

}  // namespace ui
