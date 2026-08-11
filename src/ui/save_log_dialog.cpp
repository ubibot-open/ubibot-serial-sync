#include "ui/save_log_dialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

SaveLogDialog::SaveLogDialog(const QString &suggestedBaseName, const QString &initialDirectory,
                              QWidget *parent)
    : QDialog(parent) {
    buildUi(suggestedBaseName, initialDirectory);
    retranslateUi();
}

void SaveLogDialog::buildUi(const QString &suggestedBaseName, const QString &initialDirectory) {
    auto *root = new QVBoxLayout(this);

    auto *form = new QFormLayout;
    fileNameLabel_ = new QLabel;
    fileNameEdit_ = new QLineEdit(suggestedBaseName + QStringLiteral(".log"));
    form->addRow(fileNameLabel_, fileNameEdit_);

    locationLabel_ = new QLabel;
    auto *locationRow = new QWidget;
    auto *locationLayout = new QHBoxLayout(locationRow);
    locationLayout->setContentsMargins(0, 0, 0, 0);
    locationEdit_ = new QLineEdit(initialDirectory);
    browseButton_ = new QPushButton;
    connect(browseButton_, &QPushButton::clicked, this, &SaveLogDialog::browseForDirectory);
    locationLayout->addWidget(locationEdit_, 1);
    locationLayout->addWidget(browseButton_);
    form->addRow(locationLabel_, locationRow);

    formatGroup_ = new QGroupBox;
    auto *formatLayout = new QHBoxLayout(formatGroup_);
    plainRadio_ = new QRadioButton;
    plainRadio_->setChecked(true);
    csvRadio_ = new QRadioButton(QStringLiteral("CSV"));
    hexRadio_ = new QRadioButton;
    formatLayout->addWidget(plainRadio_);
    formatLayout->addWidget(csvRadio_);
    formatLayout->addWidget(hexRadio_);

    autoRotateCheck_ = new QCheckBox;

    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);

    root->addLayout(form);
    root->addWidget(formatGroup_);
    root->addWidget(autoRotateCheck_);
    root->addWidget(buttonBox_);
}

void SaveLogDialog::browseForDirectory() {
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a folder"), locationEdit_->text());
    if (!dir.isEmpty()) locationEdit_->setText(dir);
}

QString SaveLogDialog::extensionForFormat() const {
    if (csvRadio_->isChecked()) return QStringLiteral(".csv");
    if (hexRadio_->isChecked()) return QStringLiteral(".hex.txt");
    return QStringLiteral(".log");
}

QString SaveLogDialog::directory() const { return locationEdit_->text(); }

QString SaveLogDialog::baseFileName() const {
    QString name = fileNameEdit_->text();
    const int dot = name.lastIndexOf(QLatin1Char('.'));
    if (dot > 0) name.truncate(dot);
    return name;
}

QString SaveLogDialog::filePath() const {
    return QDir(directory()).filePath(baseFileName() + extensionForFormat());
}

LogFileFormat SaveLogDialog::format() const {
    if (csvRadio_->isChecked()) return LogFileFormat::Csv;
    if (hexRadio_->isChecked()) return LogFileFormat::HexDump;
    return LogFileFormat::PlainText;
}

bool SaveLogDialog::autoRotateDaily() const { return autoRotateCheck_->isChecked(); }

void SaveLogDialog::retranslateUi() {
    setWindowTitle(tr("Save session log"));
    fileNameLabel_->setText(tr("File name"));
    locationLabel_->setText(tr("Location"));
    browseButton_->setText(tr("Browse…"));
    formatGroup_->setTitle(tr("Format"));
    plainRadio_->setText(tr("Plain text (.log)"));
    hexRadio_->setText(tr("HEX dump"));
    autoRotateCheck_->setText(tr("Continue logging to disk, rotating the file every day"));
}

void SaveLogDialog::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QDialog::changeEvent(event);
}
