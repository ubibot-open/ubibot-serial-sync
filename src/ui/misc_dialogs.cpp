// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/misc_dialogs.h"

#include "app/app_controller.h"
#include "ui/ui_common.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <QLight/CheckBox.h>
#include <QLight/LineEdit.h>
#include <QLight/PushButton.h>

namespace ui {

namespace {

// Source text of DriverCheck::knownDrivers()'s descriptions, so lupdate
// collects them under this context (the core keeps them untranslated).
[[maybe_unused]] const char *const kDriverDescriptions[] = {
    QT_TRANSLATE_NOOP("ui::DriverListDialog", "USB-to-serial driver used by most UbiBot devices"),
};

QLabel *linkLabel(QWidget *parent) {
    auto *l = new QLabel(parent);
    l->setTextFormat(Qt::RichText);
    l->setOpenExternalLinks(true);
    l->setTextInteractionFlags(Qt::TextBrowserInteraction);
    return l;
}

QString link(const QString &url, const QString &text) {
    return QStringLiteral("<a href=\"%1\" style=\"color:%3; text-decoration:none;\">%2</a>")
        .arg(url, text.toHtmlEscaped(), colors().accentText.name());
}

}  // namespace

// --- SaveLogDialog -------------------------------------------------------

SaveLogDialog::SaveLogDialog(AppController *controller, QWidget *parent)
    : AppDialog(parent), controller_(controller) {
    setDialogWidth(480);
    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(12);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);
    fileLabel_ = new ThemedLabel(ThemedLabel::Primary, content);
    locationLabel_ = new ThemedLabel(ThemedLabel::Primary, content);
    formatLabel_ = new ThemedLabel(ThemedLabel::Primary, content);
    fileField_ = new QLight::LineEdit(content);
    fileField_->setFont(monoFont());
    locationField_ = new QLight::LineEdit(content);
    locationField_->setFont(monoFont());
    browseButton_ = new QLight::PushButton(content);
    format_ = new SegmentedControl(content);
    format_->setOptions({QString(), QStringLiteral("CSV"), QString()});
    format_->setSegmentWidth(96);
    auto *locationRow = new QHBoxLayout;
    locationRow->setSpacing(8);
    locationRow->addWidget(locationField_, 1);
    locationRow->addWidget(browseButton_);
    grid->addWidget(fileLabel_, 0, 0);
    grid->addWidget(fileField_, 0, 1);
    grid->addWidget(locationLabel_, 1, 0);
    grid->addLayout(locationRow, 1, 1);
    grid->addWidget(formatLabel_, 2, 0);
    grid->addWidget(format_, 2, 1, Qt::AlignLeft);
    grid->setColumnStretch(1, 1);
    col->addLayout(grid);

    autoRotate_ = new QLight::CheckBox(content);
    col->addWidget(autoRotate_);
    errorLabel_ = new ThemedLabel(ThemedLabel::Danger, content);
    errorLabel_->setWordWrap(true);
    errorLabel_->hide();
    col->addWidget(errorLabel_);
    setContentWidget(content);

    cancelButton_ = addRejectButton(QString());
    saveButton_ = addActionButton(QString(), true);
    connect(saveButton_, &QPushButton::clicked, this, &SaveLogDialog::save);
    connect(browseButton_, &QPushButton::clicked, this, [this] {
        const QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a folder"), locationField_->text());
        if (!dir.isEmpty()) locationField_->setText(QDir::toNativeSeparators(dir));
    });
    retranslateUi();
}

void SaveLogDialog::retranslateUi() {
    setTitle(tr("Save session log"));
    fileLabel_->setText(tr("File name"));
    locationLabel_->setText(tr("Location"));
    formatLabel_->setText(tr("Format"));
    browseButton_->setText(tr("Browse…"));
    format_->setOptionText(0, tr("Plain text (.log)"));
    format_->setOptionText(2, tr("HEX dump"));
    autoRotate_->setText(tr("Continue logging to disk, rotating the file every day"));
    cancelButton_->setText(tr("Cancel"));
    saveButton_->setText(tr("Save"));
}

void SaveLogDialog::openDialog() {
    fileField_->setText(controller_->suggestedLogBaseName() + QStringLiteral(".log"));
    locationField_->setText(QDir::toNativeSeparators(controller_->suggestedLogDirectory()));
    format_->setCurrentIndex(0);
    autoRotate_->setChecked(false);
    errorLabel_->hide();
    open();
}

void SaveLogDialog::save() {
    QString baseName = fileField_->text().trimmed();
    const int dot = baseName.lastIndexOf(QLatin1Char('.'));
    if (dot > 0) baseName = baseName.left(dot);
    static const char *const formats[] = {"text", "csv", "hex"};
    const QString format = QString::fromLatin1(formats[qBound(0, format_->currentIndex(), 2)]);
    const QString error = controller_->saveLog(QDir::fromNativeSeparators(locationField_->text().trimmed()),
                                               baseName, format, autoRotate_->isChecked());
    if (!error.isEmpty()) {
        errorLabel_->setText(error);
        errorLabel_->show();
        refit();
        return;
    }
    accept();
}

// --- AboutDialog ---------------------------------------------------------

AboutDialog::AboutDialog(AppController *controller, QWidget *parent) : AppDialog(parent), controller_(controller) {
    setDialogWidth(460);
    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(12);

    auto *head = new QHBoxLayout;
    head->setSpacing(12);
    auto *logo = new QLabel(content);
    logo->setPixmap(QIcon(QStringLiteral(":/icons/app.png")).pixmap(40, 40));
    logo->setFixedSize(40, 40);
    auto *names = new QVBoxLayout;
    names->setSpacing(2);
    name_ = new ThemedLabel(ThemedLabel::Primary, content);
    name_->setEmphasis(true, 2);
    version_ = new ThemedLabel(ThemedLabel::Secondary, content);
    names->addWidget(name_);
    names->addWidget(version_);
    head->addWidget(logo);
    head->addLayout(names, 1);
    col->addLayout(head);

    description_ = new ThemedLabel(ThemedLabel::Secondary, content);
    description_->setWordWrap(true);
    col->addWidget(description_);
    col->addWidget(new Divider(Qt::Horizontal, content));

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(8);
    buildTimeLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    buildTime_ = new ThemedLabel(controller_->buildTime(), ThemedLabel::Primary, content);
    companyLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    company_ = new ThemedLabel(ThemedLabel::Primary, content);
    websiteLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    website_ = linkLabel(content);
    githubLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    github_ = linkLabel(content);
    grid->addWidget(buildTimeLabel_, 0, 0);
    grid->addWidget(buildTime_, 0, 1);
    grid->addWidget(companyLabel_, 1, 0);
    grid->addWidget(company_, 1, 1);
    grid->addWidget(websiteLabel_, 2, 0);
    grid->addWidget(website_, 2, 1);
    grid->addWidget(githubLabel_, 3, 0);
    grid->addWidget(github_, 3, 1);
    grid->setColumnStretch(1, 1);
    col->addLayout(grid);
    col->addWidget(new Divider(Qt::Horizontal, content));

    license_ = new ThemedLabel(ThemedLabel::Secondary, content);
    license_->setWordWrap(true);
    license_->setEmphasis(false, -1);
    col->addWidget(license_);
    setContentWidget(content);

    closeButton_ = addAcceptButton(QString());
    onThemeChanged(this, [this] {
        website_->setText(link(QStringLiteral("https://www.ubibot.com"), QStringLiteral("https://www.ubibot.com")));
        github_->setText(link(QStringLiteral("https://github.com/ubibot-open/ubibot-serial-sync"),
                              QStringLiteral("github.com/ubibot-open/ubibot-serial-sync")));
    });
    retranslateUi();
}

void AboutDialog::retranslateUi() {
    setTitle(tr("About"));
    name_->setText(tr("UbiBot Serial Assistant"));
    version_->setText(tr("Version %1").arg(controller_->appVersion()));
    description_->setText(tr("A serial-port debugging tool for UbiBot IoT devices (WS1, WS1 Pro, GS1-AL4G1RS, SP1, "
                             "and more), built with Qt 6 and C++17."));
    buildTimeLabel_->setText(tr("Build time"));
    companyLabel_->setText(tr("Company"));
    company_->setText(tr("UbiBot · United States of America"));
    websiteLabel_->setText(tr("Website"));
    githubLabel_->setText(tr("GitHub"));
    license_->setText(tr("Built with Qt %1 under the GNU Lesser General Public License v3 (LGPLv3). Qt is a trademark "
                         "of The Qt Company Ltd. Qt is linked dynamically, so the bundled Qt libraries may be "
                         "replaced with a compatible LGPLv3 build.")
                          .arg(controller_->qtVersion()));
    closeButton_->setText(tr("Close"));
}

// --- DriverHintDialog ----------------------------------------------------

DriverHintDialog::DriverHintDialog(AppController *controller, QWidget *parent)
    : AppDialog(parent), controller_(controller) {
    setDialogWidth(440);
    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(14);
    text_ = new ThemedLabel(ThemedLabel::Primary, content);
    text_->setWordWrap(true);
    dontShow_ = new QLight::CheckBox(content);
    dontShow_->setChecked(controller_->ch340DriverHintDismissed());
    col->addWidget(text_);
    col->addWidget(dontShow_);
    setContentWidget(content);

    downloadButton_ = addActionButton(QString(), true);
    closeButton_ = addRejectButton(QString());
    connect(downloadButton_, &QPushButton::clicked, this, [this] {
        accept();
        emit downloadRequested();
    });
    connect(dontShow_, &QCheckBox::toggled, controller_, &AppController::setCh340DriverHintDismissed);
    connect(controller_, &AppController::ch340DriverHintDismissedChanged, this, [this] {
        dontShow_->setChecked(controller_->ch340DriverHintDismissed());
    });
    retranslateUi();
}

void DriverHintDialog::retranslateUi() {
    setTitle(tr("Serial driver may be missing"));
    text_->setText(tr("The CH340 USB-to-serial driver doesn't appear to be installed on this computer. Without it, a "
                      "connected UbiBot device won't show up as an available serial port."));
    dontShow_->setText(tr("Don't show this again"));
    downloadButton_->setText(tr("Download driver"));
    closeButton_->setText(tr("Close"));
}

// --- DriverListDialog ----------------------------------------------------

DriverListDialog::DriverListDialog(AppController *controller, QWidget *parent)
    : AppDialog(parent), controller_(controller) {
    setDialogWidth(460);
    auto *content = new QWidget(this);
    rows_ = new QVBoxLayout(content);
    rows_->setContentsMargins(0, 0, 0, 0);
    rows_->setSpacing(0);
    setContentWidget(content);
    closeButton_ = addAcceptButton(QString());
    retranslateUi();
}

void DriverListDialog::retranslateUi() {
    setTitle(tr("Driver downloads"));
    closeButton_->setText(tr("Close"));
    rebuild();
}

void DriverListDialog::openDialog() {
    rebuild();
    open();
}

void DriverListDialog::rebuild() {
    QWidget *host = contentWidget();
    while (QLayoutItem *item = rows_->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
    const QVariantList drivers = controller_->availableDrivers();
    if (drivers.isEmpty()) {
        auto *none = new ThemedLabel(tr("No drivers are relevant on this platform."), ThemedLabel::Secondary, host);
        none->setWordWrap(true);
        rows_->addWidget(none);
        return;
    }
    for (int i = 0; i < drivers.size(); ++i) {
        const QVariantMap d = drivers.at(i).toMap();
        if (i > 0) rows_->addWidget(new Divider(Qt::Horizontal, host));
        auto *row = new QWidget(host);
        auto *h = new QHBoxLayout(row);
        h->setContentsMargins(0, 10, 0, 10);
        h->setSpacing(12);
        auto *texts = new QVBoxLayout;
        texts->setSpacing(2);
        auto *name = new ThemedLabel(d.value(QStringLiteral("name")).toString(), ThemedLabel::Primary, row);
        name->setEmphasis(true);
        auto *desc = new ThemedLabel(
            QCoreApplication::translate("ui::DriverListDialog",
                                        d.value(QStringLiteral("description")).toString().toUtf8().constData()),
            ThemedLabel::Secondary, row);
        desc->setWordWrap(true);
        const bool installed = d.value(QStringLiteral("installed")).toBool();
        auto *state = new ThemedLabel(installed ? tr("Installed") : tr("Not detected"),
                                      installed ? ThemedLabel::Success : ThemedLabel::Danger, row);
        texts->addWidget(name);
        texts->addWidget(desc);
        texts->addWidget(state);
        h->addLayout(texts, 1);
        const QString url = d.value(QStringLiteral("url")).toString();
        if (!url.isEmpty()) {
            auto *download = new QLight::PushButton(tr("Download"), QLight::PushButton::Accent, row);
            connect(download, &QPushButton::clicked, this, [url] { openUrl(url); });
            h->addWidget(download, 0, Qt::AlignVCenter);
        }
        rows_->addWidget(row);
    }
}

}  // namespace ui
