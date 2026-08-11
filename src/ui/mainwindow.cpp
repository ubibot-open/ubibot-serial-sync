#include "ui/mainwindow.h"

#include "core/language_manager.h"
#include "ui/command_library_panel.h"
#include "ui/command_params_panel.h"
#include "ui/connection_wizard.h"
#include "ui/data_monitor_view.h"
#include "ui/remote_assist_panel.h"
#include "ui/save_log_dialog.h"
#include "ui/serial_settings_panel.h"
#include "ui/settings_about_dialog.h"
#include "ui/styles.h"

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QDateTime>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

namespace {
constexpr int kCmdModeIndex = 0;
constexpr int kSerialModeIndex = 1;
constexpr int kRemoteModeIndex = 2;
}  // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    library_.loadFromResource();
    LanguageManager::instance().setLanguage(settings_.language());

    repeatTimer_ = new QTimer(this);
    connect(repeatTimer_, &QTimer::timeout, this, &MainWindow::sendManualInput);

    buildUi();
    buildMenus();
    buildToolbar();
    buildStatusBar();
    wireSignals();
    retranslateUi();

    commandPanel_->setCurrentModelId(settings_.lastModelId().isEmpty() ? library_.modelIds().value(0)
                                                                        : settings_.lastModelId());
    serialPanel_->setConfig(settings_.lastSerialConfig());

    resize(1200, 800);
    if (!settings_.windowGeometry().isEmpty()) restoreGeometry(settings_.windowGeometry());

    updateConnectionUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    auto *central = new QWidget;
    auto *centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    // --- mode selector row (segmented control) --------------------------
    auto *modeRow = new QWidget;
    modeRow->setStyleSheet(QStringLiteral("border-bottom: 1px solid #c9c9ca;"));
    auto *modeRowLayout = new QHBoxLayout(modeRow);
    modeRowLayout->setContentsMargins(12, 8, 12, 8);
    cmdModeButton_ = new QPushButton;
    serialModeButton_ = new QPushButton;
    remoteModeButton_ = new QPushButton;
    modeButtons_ = new QButtonGroup(this);
    modeButtons_->setExclusive(true);
    for (auto *btn : {cmdModeButton_, serialModeButton_, remoteModeButton_}) btn->setCheckable(true);
    modeButtons_->addButton(cmdModeButton_, kCmdModeIndex);
    modeButtons_->addButton(serialModeButton_, kSerialModeIndex);
    modeButtons_->addButton(remoteModeButton_, kRemoteModeIndex);
    cmdModeButton_->setChecked(true);
    modeRowLayout->addWidget(cmdModeButton_);
    modeRowLayout->addWidget(serialModeButton_);
    modeRowLayout->addWidget(remoteModeButton_);

    // Right-aligned "current device" badge + open/close button, mirroring
    // the design's top toolbar row.
    modeRowLayout->addStretch();
    auto *badge = new QVBoxLayout;
    badge->setSpacing(0);
    modelBadgeValue_ = new QLabel;
    modelBadgeValue_->setStyleSheet(QStringLiteral("font-family: Consolas, monospace; font-size: 12px;"));
    modelBadgeValue_->setAlignment(Qt::AlignRight);
    modelBadgeCaption_ = new QLabel;
    modelBadgeCaption_->setStyleSheet(QStringLiteral("font-size: 10px; color: #7a7a7d;"));
    modelBadgeCaption_->setAlignment(Qt::AlignRight);
    badge->addWidget(modelBadgeValue_);
    badge->addWidget(modelBadgeCaption_);
    modeRowLayout->addLayout(badge);
    portToggleButton_ = new QPushButton;
    portToggleButton_->setProperty("primary", true);
    portToggleButton_->setMinimumWidth(120);
    modeRowLayout->addWidget(portToggleButton_);

    // --- body: left mode-specific panel + right data monitor -------------
    auto *body = new QWidget;
    auto *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    modeStack_ = new QStackedWidget;
    modeStack_->setFixedWidth(330);
    commandPanel_ = new CommandLibraryPanel(&library_, &settings_);
    serialPanel_ = new SerialSettingsPanel;
    remotePanel_ = new RemoteAssistPanel;
    modeStack_->insertWidget(kCmdModeIndex, commandPanel_);
    modeStack_->insertWidget(kSerialModeIndex, serialPanel_);
    modeStack_->insertWidget(kRemoteModeIndex, remotePanel_);

    auto *leftFrame = new QWidget;
    leftFrame->setStyleSheet(QStringLiteral("border-right: 1px solid #c9c9ca;"));
    auto *leftLayout = new QVBoxLayout(leftFrame);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addWidget(modeStack_);

    auto *rightFrame = new QWidget;
    auto *rightLayout = new QVBoxLayout(rightFrame);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    monitorView_ = new DataMonitorView(&logManager_);
    paramsPanel_ = new CommandParamsPanel;

    auto *inputRow = new QWidget;
    auto *inputRowLayout = new QHBoxLayout(inputRow);
    inputRowLayout->setContentsMargins(14, 12, 14, 12);
    inputEdit_ = new QTextEdit;
    inputEdit_->setFixedHeight(74);
    inputEdit_->setStyleSheet(QStringLiteral("font-family: Consolas, monospace; font-size: 13px;"));
    auto *inputButtonsCol = new QVBoxLayout;
    sendButton_ = new QPushButton;
    sendButton_->setProperty("primary", true);
    clearInputButton_ = new QPushButton;
    inputButtonsCol->addWidget(sendButton_);
    inputButtonsCol->addWidget(clearInputButton_);
    inputRowLayout->addWidget(inputEdit_, 1);
    inputRowLayout->addLayout(inputButtonsCol);

    rightLayout->addWidget(monitorView_, 1);
    rightLayout->addWidget(paramsPanel_);
    rightLayout->addWidget(inputRow);

    bodyLayout->addWidget(leftFrame);
    bodyLayout->addWidget(rightFrame, 1);

    centralLayout->addWidget(modeRow);
    centralLayout->addWidget(body, 1);
    setCentralWidget(central);
}

void MainWindow::buildMenus() {
    wizardAction_ = new QAction(QIcon(QStringLiteral(":/icons/wizard.svg")), QString(), this);
    saveLogAction_ = new QAction(QIcon(QStringLiteral(":/icons/save.svg")), QString(), this);
    sendAction_ = new QAction(QIcon(QStringLiteral(":/icons/send.svg")), QString(), this);
    pauseAction_ = new QAction(QIcon(QStringLiteral(":/icons/pause.svg")), QString(), this);
    pauseAction_->setCheckable(true);
    clearAction_ = new QAction(QIcon(QStringLiteral(":/icons/clear.svg")), QString(), this);
    settingsAction_ = new QAction(QIcon(QStringLiteral(":/icons/settings.svg")), QString(), this);
    exitAction_ = new QAction(this);
    aboutQtAction_ = new QAction(this);

    connect(wizardAction_, &QAction::triggered, this, &MainWindow::openWizardDialog);
    connect(saveLogAction_, &QAction::triggered, this, &MainWindow::openSaveLogDialog);
    connect(sendAction_, &QAction::triggered, this, &MainWindow::sendManualInput);
    connect(pauseAction_, &QAction::toggled, this, [this](bool paused) { monitorView_->setPaused(paused); });
    connect(clearAction_, &QAction::triggered, monitorView_, &DataMonitorView::clearLog);
    connect(settingsAction_, &QAction::triggered, this, &MainWindow::openSettingsDialog);
    connect(exitAction_, &QAction::triggered, this, &QWidget::close);
    connect(aboutQtAction_, &QAction::triggered, this, [this] { QApplication::aboutQt(); });

    fileMenu_ = menuBar()->addMenu(QString());
    fileMenu_->addAction(wizardAction_);
    fileMenu_->addAction(saveLogAction_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(exitAction_);

    editMenu_ = menuBar()->addMenu(QString());
    editMenu_->addAction(clearAction_);

    viewMenu_ = menuBar()->addMenu(QString());
    viewMenu_->addAction(pauseAction_);

    toolsMenu_ = menuBar()->addMenu(QString());
    toolsMenu_->addAction(settingsAction_);

    helpMenu_ = menuBar()->addMenu(QString());
    helpMenu_->addAction(settingsAction_);
    helpMenu_->addAction(aboutQtAction_);
}

void MainWindow::buildToolbar() {
    auto *toolbar = addToolBar(QStringLiteral("main"));
    toolbar->setMovable(false);
    toolbar->addAction(wizardAction_);
    toolbar->addAction(saveLogAction_);
    toolbar->addSeparator();
    toolbar->addAction(sendAction_);
    toolbar->addAction(pauseAction_);
    toolbar->addAction(clearAction_);
    toolbar->addSeparator();
    toolbar->addAction(settingsAction_);
}

void MainWindow::buildStatusBar() {
    connectionStatusLabel_ = new QLabel;
    portParamsLabel_ = new QLabel;
    byteCountersLabel_ = new QLabel;
    for (QLabel *l : {connectionStatusLabel_, portParamsLabel_, byteCountersLabel_})
        l->setStyleSheet(QStringLiteral("font-family: Consolas, monospace; font-size: 11px;"));
    statusBar()->addWidget(connectionStatusLabel_);
    statusBar()->addWidget(portParamsLabel_);
    statusBar()->addPermanentWidget(byteCountersLabel_);
}

void MainWindow::wireSignals() {
    connect(modeButtons_, &QButtonGroup::idClicked, modeStack_, &QStackedWidget::setCurrentIndex);

    connect(&serial_, &SerialManager::dataReceived, this,
            [this](const QByteArray &data) { logManager_.append(LogKind::Rx, data); });
    connect(&serial_, &SerialManager::errorOccurred, this, [this](const QString &msg) {
        logManager_.append(LogKind::Err, msg.toUtf8());
    });
    connect(&serial_, &SerialManager::opened, this, [this](const SerialConfig &) { updateConnectionUi(); });
    connect(&serial_, &SerialManager::closed, this, [this] { updateConnectionUi(); });
    connect(&logManager_, &LogManager::countersChanged, this, [this](qint64 rx, qint64 tx) {
        byteCountersLabel_->setText(tr("Rx %1 B   Tx %2 B").arg(rx).arg(tx));
    });

    connect(portToggleButton_, &QPushButton::clicked, this, &MainWindow::togglePort);

    connect(serialPanel_, &SerialSettingsPanel::receiveFormatChanged, monitorView_,
            &DataMonitorView::setHexMode);
    connect(serialPanel_, &SerialSettingsPanel::showTimestampChanged, monitorView_,
            &DataMonitorView::setShowTimestamp);
    connect(serialPanel_, &SerialSettingsPanel::repeatSendToggled, this, [this](bool enabled, int ms) {
        if (enabled)
            repeatTimer_->start(ms);
        else
            repeatTimer_->stop();
    });

    connect(commandPanel_, &CommandLibraryPanel::commandActivated, this,
            [this](const DeviceCommand &cmd) { sendLiteral(cmd.cmdTemplate); });
    connect(commandPanel_, &CommandLibraryPanel::commandParamsRequested, this,
            [this](const DeviceCommand &cmd) { paramsPanel_->setCommand(cmd); });
    connect(commandPanel_, &CommandLibraryPanel::modelChanged, this, [this](const QString &id) {
        settings_.setLastModelId(id);
        modelBadgeValue_->setText(id);
    });

    connect(paramsPanel_, &CommandParamsPanel::sendRequested, this, [this](const QString &resolved) {
        sendLiteral(resolved);
        paramsPanel_->setVisible(false);
    });
    connect(paramsPanel_, &CommandParamsPanel::cancelled, this, [this] { paramsPanel_->setVisible(false); });

    connect(sendButton_, &QPushButton::clicked, this, &MainWindow::sendManualInput);
    connect(clearInputButton_, &QPushButton::clicked, inputEdit_, &QTextEdit::clear);

    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this,
            [this](AppLanguage lang) { settings_.setLanguage(lang); });
}

QByteArray MainWindow::composeAsciiPayload(const QString &text) const {
    QByteArray bytes = text.toUtf8();
    if (!bytes.endsWith('\n')) bytes += "\r\n";
    return bytes;
}

QByteArray MainWindow::composeHexPayload(const QString &text) const {
    QString cleaned = text;
    cleaned.remove(QLatin1Char(' '));
    cleaned.remove(QLatin1Char('\n'));
    cleaned.remove(QLatin1Char('\r'));
    cleaned.remove(QLatin1Char('\t'));
    return QByteArray::fromHex(cleaned.toUtf8());
}

void MainWindow::sendLiteral(const QString &text) {
    if (!serial_.isOpen()) {
        logManager_.append(LogKind::Err,
                            tr("Port is not open — click \"Open port\" first.").toUtf8());
        return;
    }
    serial_.write(composeAsciiPayload(text));
    if (serialPanel_->echoTx()) logManager_.append(LogKind::Tx, text.toUtf8());
}

void MainWindow::sendManualInput() {
    const QString text = inputEdit_->toPlainText();
    if (text.trimmed().isEmpty()) return;

    if (!serial_.isOpen()) {
        logManager_.append(LogKind::Err,
                            tr("Port is not open — click \"Open port\" first.").toUtf8());
        return;
    }

    if (serialPanel_->sendAsHex()) {
        const QByteArray payload = composeHexPayload(text);
        serial_.write(payload);
        if (serialPanel_->echoTx()) logManager_.append(LogKind::Tx, payload.toHex(' ').toUpper());
    } else {
        serial_.write(composeAsciiPayload(text));
        if (serialPanel_->echoTx()) logManager_.append(LogKind::Tx, text.toUtf8());
    }
}

void MainWindow::togglePort() {
    if (serial_.isOpen()) {
        serial_.close();
        logManager_.append(LogKind::Sys, tr("Port closed").toUtf8());
        return;
    }

    const SerialConfig cfg = serialPanel_->currentConfig();
    if (cfg.portName.isEmpty()) {
        QMessageBox::warning(this, tr("UbiBot Serial Assistant"), tr("Select a serial port first."));
        return;
    }
    QString error;
    if (!serial_.open(cfg, &error)) {
        QMessageBox::warning(this, tr("Failed to open port"), error);
        return;
    }
    settings_.setLastSerialConfig(cfg);
    logManager_.append(LogKind::Sys,
                        tr("%1 opened · %2").arg(cfg.portName, serial_.portSummary()).toUtf8());
}

void MainWindow::updateConnectionUi() {
    const bool open = serial_.isOpen();
    if (!open) repeatTimer_->stop();

    portToggleButton_->setText(open ? tr("Close port") : tr("Open port"));
    serialPanel_->setEnabledWhileConnected(open);

    const QString portName = serial_.currentConfig().portName;
    const QString portLabel = portName.isEmpty() ? QStringLiteral("—") : portName;
    connectionStatusLabel_->setText(open ? tr("%1 OPEN").arg(portLabel) : tr("%1 CLOSED").arg(portLabel));
    connectionStatusLabel_->setStyleSheet(
        QStringLiteral("font-family: Consolas, monospace; font-size: 11px; color: %1;")
            .arg(open ? QStringLiteral("#416180") : QStringLiteral("#aa3333")));
    portParamsLabel_->setText(open ? serial_.portSummary() : QStringLiteral("—"));
}

void MainWindow::openWizardDialog() {
    ConnectionWizard wizard(&library_, this);
    if (wizard.exec() != QDialog::Accepted) return;

    const QString port = wizard.selectedPort();
    const QString model = wizard.selectedModelId();
    if (port.isEmpty() || model.isEmpty()) return;

    SerialConfig cfg;
    cfg.portName = port;
    serialPanel_->setConfig(cfg);
    commandPanel_->setCurrentModelId(model);
    cmdModeButton_->setChecked(true);
    modeStack_->setCurrentIndex(kCmdModeIndex);

    QString error;
    if (serial_.open(cfg, &error)) {
        settings_.setLastSerialConfig(cfg);
        logManager_.append(LogKind::Sys,
                            tr("Wizard finished · %1 opened · %2").arg(port, model).toUtf8());
    } else {
        QMessageBox::warning(this, tr("Failed to open port"), error);
    }
}

void MainWindow::openSaveLogDialog() {
    QString modelPart = commandPanel_->currentModelId();
    modelPart.replace(QLatin1Char(' '), QLatin1Char('-'));
    if (modelPart.isEmpty()) modelPart = QStringLiteral("device");

    const QString suggestedBase =
        QStringLiteral("ubibot-%1-%2").arg(modelPart, QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmm")));

    QString initialDir = settings_.lastLogDirectory();
    if (initialDir.isEmpty())
        initialDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QStringLiteral("/UbiBot");

    SaveLogDialog dialog(suggestedBase, initialDir, this);
    if (dialog.exec() != QDialog::Accepted) return;

    QString error;
    if (!logManager_.exportToFile(dialog.filePath(), dialog.format(), &error)) {
        QMessageBox::warning(this, tr("Failed to save log"), error);
        return;
    }
    settings_.setLastLogDirectory(dialog.directory());
    settings_.setContinuousLoggingEnabled(dialog.autoRotateDaily());
    logManager_.setContinuousLogging(dialog.autoRotateDaily(), dialog.directory(), dialog.baseFileName());

    statusBar()->showMessage(tr("Log saved to %1").arg(dialog.filePath()), 5000);
}

void MainWindow::openSettingsDialog() {
    SettingsAboutDialog dialog(&library_, this);
    dialog.exec();
}

void MainWindow::retranslateUi() {
    setWindowTitle(tr("UbiBot Serial Assistant"));

    cmdModeButton_->setText(tr("Device commands"));
    serialModeButton_->setText(tr("Serial"));
    remoteModeButton_->setText(tr("Remote support"));

    modelBadgeCaption_->setText(tr("Current device"));
    modelBadgeValue_->setText(commandPanel_->currentModelId());

    sendButton_->setText(tr("Send"));
    clearInputButton_->setText(tr("Clear"));
    inputEdit_->setPlaceholderText(tr("Type data to send…"));

    wizardAction_->setText(tr("Connection wizard"));
    wizardAction_->setToolTip(tr("Connection wizard"));
    saveLogAction_->setText(tr("Save log"));
    saveLogAction_->setToolTip(tr("Save log"));
    sendAction_->setText(tr("Send"));
    sendAction_->setToolTip(tr("Send"));
    pauseAction_->setText(tr("Pause scrolling"));
    pauseAction_->setToolTip(tr("Pause scrolling"));
    clearAction_->setText(tr("Clear"));
    clearAction_->setToolTip(tr("Clear"));
    settingsAction_->setText(tr("Settings"));
    settingsAction_->setToolTip(tr("Settings"));
    exitAction_->setText(tr("Exit"));
    aboutQtAction_->setText(tr("About Qt"));

    fileMenu_->setTitle(tr("&File"));
    editMenu_->setTitle(tr("&Edit"));
    viewMenu_->setTitle(tr("&View"));
    toolsMenu_->setTitle(tr("&Tools"));
    helpMenu_->setTitle(tr("&Help"));

    updateConnectionUi();
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    settings_.setWindowGeometry(saveGeometry());
    QMainWindow::closeEvent(event);
}
