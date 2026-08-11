#include "ui/settings_about_dialog.h"
#include "core/language_manager.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
constexpr auto kAppVersion = "1.0.0";
constexpr auto kQtBuildInfo = "Qt 6.11";
constexpr auto kSupportEmail = "support@ubibot.com";
}  // namespace

SettingsAboutDialog::SettingsAboutDialog(const DeviceLibrary *library, QWidget *parent)
    : QDialog(parent), library_(library) {
    buildUi();
    retranslateUi();
}

void SettingsAboutDialog::buildUi() {
    auto *root = new QVBoxLayout(this);

    languageGroup_ = new QGroupBox;
    auto *langLayout = new QHBoxLayout(languageGroup_);
    languageCombo_ = new QComboBox;
    for (const LanguageInfo &lang : LanguageManager::availableLanguages())
        languageCombo_->addItem(lang.nativeName, lang.code);
    const int currentIdx = languageCombo_->findData(LanguageManager::instance().language());
    languageCombo_->setCurrentIndex(currentIdx >= 0 ? currentIdx : 0);
    connect(languageCombo_, &QComboBox::currentIndexChanged, this, [this](int index) {
        LanguageManager::instance().setLanguage(languageCombo_->itemData(index).toString());
    });
    langLayout->addWidget(languageCombo_);
    langLayout->addStretch();

    libraryGroup_ = new QGroupBox;
    auto *libLayout = new QHBoxLayout(libraryGroup_);
    libraryVersionLabel_ = new QLabel;
    libraryVersionLabel_->setStyleSheet(QStringLiteral("font-family: Consolas, monospace;"));
    libraryStatusTag_ = new QLabel;
    libraryStatusTag_->setStyleSheet(
        QStringLiteral("background: #eef6ff; color: #2c455d; padding: 2px 8px; font-size: 11px;"));
    checkUpdateButton_ = new QPushButton;
    connect(checkUpdateButton_, &QPushButton::clicked, this, &SettingsAboutDialog::checkForUpdate);
    libLayout->addWidget(libraryVersionLabel_);
    libLayout->addWidget(libraryStatusTag_);
    libLayout->addStretch();
    libLayout->addWidget(checkUpdateButton_);

    auto *infoGrid = new QGridLayout;
    versionCaption_ = new QLabel;
    versionValue_ = new QLabel;
    platformCaption_ = new QLabel;
    platformValue_ = new QLabel(QStringLiteral("Windows · macOS · Linux"));
    supportCaption_ = new QLabel;
    supportValue_ = new QLabel(QString::fromUtf8(kSupportEmail));
    devicesCaption_ = new QLabel;
    devicesValue_ = new QLabel;
    for (QLabel *caption : {versionCaption_, platformCaption_, supportCaption_, devicesCaption_})
        caption->setStyleSheet(QStringLiteral("color: #7a7a7d; font-size: 11px;"));
    infoGrid->addWidget(versionCaption_, 0, 0);
    infoGrid->addWidget(versionValue_, 1, 0);
    infoGrid->addWidget(platformCaption_, 0, 1);
    infoGrid->addWidget(platformValue_, 1, 1);
    infoGrid->addWidget(supportCaption_, 2, 0);
    infoGrid->addWidget(supportValue_, 3, 0);
    infoGrid->addWidget(devicesCaption_, 2, 1);
    infoGrid->addWidget(devicesValue_, 3, 1);

    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);

    root->addWidget(languageGroup_);
    root->addWidget(libraryGroup_);
    root->addLayout(infoGrid);
    root->addStretch();
    root->addWidget(buttonBox_);
}

void SettingsAboutDialog::checkForUpdate() {
    // No update server is part of this build; report the bundled version
    // rather than pretending to reach the network.
    QMessageBox::information(this, tr("Command library"),
                              tr("Bundled command library is %1 — this build checks the local copy only.")
                                  .arg(library_->version()));
}

void SettingsAboutDialog::retranslateUi() {
    setWindowTitle(tr("Settings & About"));
    languageGroup_->setTitle(tr("Interface language"));

    libraryGroup_->setTitle(tr("Command library"));
    libraryVersionLabel_->setText(library_->version());
    libraryStatusTag_->setText(tr("Up to date"));
    checkUpdateButton_->setText(tr("Check for updates"));

    versionCaption_->setText(tr("Version"));
    versionValue_->setText(QStringLiteral("%1 (%2)").arg(kAppVersion, kQtBuildInfo));
    platformCaption_->setText(tr("Platform"));
    supportCaption_->setText(tr("Support"));
    devicesCaption_->setText(tr("Devices"));
    devicesValue_->setText(tr("%1 models · %2 commands")
                                .arg(int(library_->models().size()))
                                .arg(library_->commandCount()));
}

void SettingsAboutDialog::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QDialog::changeEvent(event);
}
