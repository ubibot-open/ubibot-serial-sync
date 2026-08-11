#include "ui/remote_assist_panel.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QVBoxLayout>

RemoteAssistPanel::RemoteAssistPanel(QWidget *parent) : QWidget(parent) {
    buildUi();
    regenerateCode();
    retranslateUi();
}

void RemoteAssistPanel::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 16, 14, 16);
    root->setSpacing(14);

    titleLabel_ = new QLabel;
    QFont titleFont = titleLabel_->font();
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);

    introLabel_ = new QLabel;
    introLabel_->setWordWrap(true);
    introLabel_->setStyleSheet(QStringLiteral("color: #5d5d60; font-size: 12px;"));

    auto *codeBox = new QWidget;
    codeBox->setStyleSheet(QStringLiteral("QWidget { border: 1px solid #c9c9ca; }"));
    auto *codeLayout = new QVBoxLayout(codeBox);
    codeLayout->setAlignment(Qt::AlignHCenter);
    codeCaptionLabel_ = new QLabel;
    codeCaptionLabel_->setAlignment(Qt::AlignHCenter);
    codeCaptionLabel_->setStyleSheet(QStringLiteral("font-size: 10px; letter-spacing: 1px; color: #7a7a7d;"));
    codeLabel_ = new QLabel;
    codeLabel_->setAlignment(Qt::AlignHCenter);
    codeLabel_->setStyleSheet(
        QStringLiteral("font-family: Consolas, monospace; font-size: 26px; letter-spacing: 3px; color: #2c455d;"));
    codeHintLabel_ = new QLabel;
    codeHintLabel_->setAlignment(Qt::AlignHCenter);
    codeHintLabel_->setStyleSheet(QStringLiteral("font-size: 11px; color: #98989b;"));
    auto *codeButtonsRow = new QHBoxLayout;
    copyButton_ = new QPushButton;
    regenerateButton_ = new QPushButton;
    connect(copyButton_, &QPushButton::clicked, this, [this] {
        QApplication::clipboard()->setText(code_);
    });
    connect(regenerateButton_, &QPushButton::clicked, this, [this] { regenerateCode(); });
    codeButtonsRow->addWidget(copyButton_);
    codeButtonsRow->addWidget(regenerateButton_);
    codeLayout->addWidget(codeCaptionLabel_);
    codeLayout->addWidget(codeLabel_);
    codeLayout->addWidget(codeHintLabel_);
    codeLayout->addLayout(codeButtonsRow);

    otpCaptionLabel_ = new QLabel;
    otpEdit_ = new QLineEdit;
    otpEdit_->setReadOnly(true);
    otpEdit_->setAlignment(Qt::AlignHCenter);
    otpEdit_->setStyleSheet(QStringLiteral("font-family: Consolas, monospace; letter-spacing: 5px;"));

    allowPortControlCheck_ = new QCheckBox;
    allowPortControlCheck_->setChecked(true);
    shareLogCheck_ = new QCheckBox;
    shareLogCheck_->setChecked(true);
    fullDesktopCheck_ = new QCheckBox;

    sessionButton_ = new QPushButton;
    sessionButton_->setProperty("primary", true);
    connect(sessionButton_, &QPushButton::clicked, this, [this] {
        QMessageBox::information(this, tr("Remote support"),
                                  tr("Remote support requires a signaling/relay service that this build "
                                     "does not include yet. The session code and permissions above are "
                                     "ready to wire up once that transport exists."));
    });

    statusLabel_ = new QLabel;
    statusLabel_->setStyleSheet(QStringLiteral("font-size: 11px; color: #7a7a7d;"));

    root->addWidget(titleLabel_);
    root->addWidget(introLabel_);
    root->addWidget(codeBox);
    root->addWidget(otpCaptionLabel_);
    root->addWidget(otpEdit_);
    root->addWidget(allowPortControlCheck_);
    root->addWidget(shareLogCheck_);
    root->addWidget(fullDesktopCheck_);
    root->addWidget(sessionButton_);
    root->addWidget(statusLabel_);
    root->addStretch();
}

void RemoteAssistPanel::regenerateCode() {
    auto group = [] { return QString::number(QRandomGenerator::global()->bounded(1000, 10000)); };
    code_ = QStringLiteral("%1-%2-%3").arg(group(), group(), group());
    otp_ = QString::number(QRandomGenerator::global()->bounded(1000, 10000));
    codeLabel_->setText(code_);
    otpEdit_->setText(otp_);
}

void RemoteAssistPanel::retranslateUi() {
    titleLabel_->setText(tr("Remote support"));
    introLabel_->setText(tr("Send the code below to UbiBot support so they can connect to your "
                             "computer and help diagnose device issues."));
    codeCaptionLabel_->setText(tr("Your code"));
    codeHintLabel_->setText(tr("Valid for 10 minutes · this session only"));
    copyButton_->setText(tr("Copy code"));
    regenerateButton_->setText(tr("Regenerate"));
    otpCaptionLabel_->setText(tr("One-time password"));
    allowPortControlCheck_->setText(tr("Allow support to send/receive on the serial port"));
    shareLogCheck_->setText(tr("Share this session's log"));
    fullDesktopCheck_->setText(tr("Allow full desktop control"));
    sessionButton_->setText(tr("Start session"));
    statusLabel_->setText(tr("Not connected"));
}

void RemoteAssistPanel::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
