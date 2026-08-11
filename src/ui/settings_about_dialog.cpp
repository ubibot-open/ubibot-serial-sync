#include "ui/settings_about_dialog.h"
#include "core/language_manager.h"

#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
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
    bilingualRadio_ = new QRadioButton;
    chineseRadio_ = new QRadioButton;
    englishRadio_ = new QRadioButton;
    switch (LanguageManager::instance().language()) {
    case AppLanguage::Chinese: chineseRadio_->setChecked(true); break;
    case AppLanguage::English: englishRadio_->setChecked(true); break;
    case AppLanguage::Bilingual: default: bilingualRadio_->setChecked(true); break;
    }
    connect(bilingualRadio_, &QRadioButton::toggled, this, [](bool on) {
        if (on) LanguageManager::instance().setLanguage(AppLanguage::Bilingual);
    });
    connect(chineseRadio_, &QRadioButton::toggled, this, [](bool on) {
        if (on) LanguageManager::instance().setLanguage(AppLanguage::Chinese);
    });
    connect(englishRadio_, &QRadioButton::toggled, this, [](bool on) {
        if (on) LanguageManager::instance().setLanguage(AppLanguage::English);
    });
    langLayout->addWidget(bilingualRadio_);
    langLayout->addWidget(chineseRadio_);
    langLayout->addWidget(englishRadio_);

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
    bilingualRadio_->setText(tr("Bilingual (Chinese + English)"));
    chineseRadio_->setText(tr("简体中文"));
    englishRadio_->setText(tr("English"));

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
