// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/main_view.h"

#include "app/app_controller.h"
#include "ui/command_library_panel.h"
#include "ui/command_params_panel.h"
#include "ui/data_monitor_view.h"
#include "ui/misc_dialogs.h"
#include "ui/serial_settings_panel.h"
#include "ui/settings_dialog.h"
#include "ui/ui_common.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <QLight/Dialog.h>
#include <QLight/FluentIcon.h>
#include <QLight/Flyout.h>
#include <QLight/InfoBar.h>
#include <QLight/Menu.h>
#include <QLight/MenuBar.h>
#include <QLight/PushButton.h>
#include <QLight/TextEdit.h>
#include <QLight/Theme.h>
#include <QLight/ToolButton.h>

namespace ui {

namespace {

constexpr int kOuterMargin = 12;
constexpr int kToastMs = 5000;

QLight::ToolButton *commandButton(QLight::Icon icon, QWidget *parent) {
    auto *b = new QLight::ToolButton(QLight::FluentIcon(icon), parent);
    b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    b->setAutoRaise(true);
    b->setFixedHeight(32);
    return b;
}

}  // namespace

MainView::MainView(AppController *controller, QWidget *parent) : QWidget(parent), controller_(controller) {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    menuBar_ = new QLight::MenuBar(this);
    buildMenus();
    root->addWidget(menuBar_);

    auto *body = new QWidget(this);
    auto *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(kOuterMargin, 4, kOuterMargin, 0);
    bodyLayout->setSpacing(10);
    bodyLayout->addWidget(buildCommandBar());

    driverBar_ = new QLight::InfoBar(QLight::InfoBar::Warning, QString(), QString(), body);
    driverBar_->setClosable(true);
    bodyLayout->addWidget(driverBar_);

    auto *panels = new QHBoxLayout;
    panels->setSpacing(kOuterMargin);
    panels->addWidget(buildLeftPanel());
    panels->addWidget(buildRightSide(), 1);
    bodyLayout->addLayout(panels, 1);
    root->addWidget(body, 1);
    root->addWidget(buildStatusBar());

    // --- controller wiring --------------------------------------------------
    connect(controller_, &AppController::connectionChanged, this, &MainView::updateConnectionState);
    connect(controller_, &AppController::selectedPortNameChanged, this, &MainView::updateConnectionState);
    connect(controller_, &AppController::statusMessage, this, &MainView::showToast);
    connect(controller_, &AppController::portOpenFailed, this, [this](const QString &error) {
        QLight::Dialog::inform(window(), tr("Failed to open port"), error, tr("OK"));
    });
    connect(controller_, &AppController::ch340DriverHintDismissedChanged, this, &MainView::updateDriverBar);
    connect(driverBar_, &QLight::InfoBar::closed, this, [this] { controller_->setCh340DriverHintDismissed(true); });
    connect(driverBar_, &QLight::InfoBar::actionTriggered, this, &MainView::openDriverHint);

    // An update found by the automatic startup check (or by any other check
    // that didn't already open the dialog) surfaces itself here -- the check
    // itself never shows UI, so this is the one place that decides to.
    connect(controller_, &AppController::appUpdateStateChanged, this, [this] {
        const QString state = controller_->appUpdateState();
        if (state != QLatin1String("updateAvailable")) return;
        if (!softwareUpdateDialog_) softwareUpdateDialog_ = new SoftwareUpdateDialog(controller_, window());
        softwareUpdateDialog_->open();
    });

    // Silent, at most once a day -- see AppController::maybeCheckForAppUpdate().
    // Delayed so it never competes with the first paint / port enumeration.
    QTimer::singleShot(5000, this, [this] { controller_->maybeCheckForAppUpdate(); });

    retranslateUi();
    updateConnectionState();
    updateDriverBar();
}

void MainView::buildMenus() {
    fileMenu_ = menuBar_->createMenu(QString());
    saveLogAction_ = fileMenu_->addAction(QLight::FluentIcon(QLight::Icon::Save).icon(), QString());
    fileMenu_->addSeparator();
    exitAction_ = fileMenu_->addAction(QString());

    editMenu_ = menuBar_->createMenu(QString());
    clearAction_ = editMenu_->addAction(QLight::FluentIcon(QLight::Icon::Delete).icon(), QString());

    viewMenu_ = menuBar_->createMenu(QString());
    pauseAction_ = viewMenu_->addAction(QString());
    pauseAction_->setCheckable(true);

    toolsMenu_ = menuBar_->createMenu(QString());
    settingsAction_ = toolsMenu_->addAction(QLight::FluentIcon(QLight::Icon::Settings).icon(), QString());

    helpMenu_ = menuBar_->createMenu(QString());
    // Checks the app itself against its GitHub Releases (see
    // core/software_update_client.h) -- a separate thing from the command
    // library update in Settings & About.
    updateAction_ =
        helpMenu_->addAction(QLight::FluentIcon(QLight::Icon::ArrowSync).icon(), QString());
    helpMenu_->addSeparator();
    driversAction_ = helpMenu_->addAction(QString());
    helpMenu_->addSeparator();
    aboutAction_ = helpMenu_->addAction(QLight::FluentIcon(QLight::Icon::Info).icon(), QString());

    connect(saveLogAction_, &QAction::triggered, this, &MainView::openSaveLog);
    connect(exitAction_, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    connect(clearAction_, &QAction::triggered, this, [this] { controller_->logModel()->clear(); });
    connect(pauseAction_, &QAction::toggled, this, [this](bool on) { monitor_->setPaused(on); });
    connect(settingsAction_, &QAction::triggered, this, &MainView::openSettings);
    connect(updateAction_, &QAction::triggered, this, &MainView::openSoftwareUpdate);
    connect(driversAction_, &QAction::triggered, this, &MainView::openDriverList);
    connect(aboutAction_, &QAction::triggered, this, &MainView::openAbout);
}

QWidget *MainView::buildCommandBar() {
    auto *bar = new QWidget(this);
    auto *row = new QHBoxLayout(bar);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(4);
    saveLogButton_ = commandButton(QLight::Icon::Save, bar);
    pauseButton_ = commandButton(QLight::Icon::Pause, bar);
    pauseButton_->setCheckable(true);
    clearButton_ = commandButton(QLight::Icon::Delete, bar);
    settingsButton_ = commandButton(QLight::Icon::Settings, bar);
    auto *sep = new Divider(Qt::Vertical, bar);
    sep->setFixedHeight(22);
    row->addWidget(saveLogButton_);
    row->addWidget(pauseButton_);
    row->addWidget(clearButton_);
    row->addSpacing(6);
    row->addWidget(sep);
    row->addSpacing(6);
    row->addWidget(settingsButton_);
    row->addStretch(1);
    statusPill_ = new StatusPill(bar);
    row->addWidget(statusPill_);

    connect(saveLogButton_, &QToolButton::clicked, this, &MainView::openSaveLog);
    connect(pauseButton_, &QToolButton::toggled, this, [this](bool on) { monitor_->setPaused(on); });
    connect(clearButton_, &QToolButton::clicked, this, [this] { controller_->logModel()->clear(); });
    connect(settingsButton_, &QToolButton::clicked, this, &MainView::openSettings);
    return bar;
}

QWidget *MainView::buildLeftPanel() {
    auto *panel = new Panel(this);
    panel->setFixedWidth(316);
    auto *col = new QVBoxLayout(panel);
    col->setContentsMargins(1, 1, 1, 1);
    col->setSpacing(0);

    modeTabs_ = new PivotBar(panel);
    modeTabs_->setTabs({QString(), QString()});
    col->addWidget(modeTabs_);
    col->addWidget(new Divider(Qt::Horizontal, panel));

    modeStack_ = new QStackedWidget(panel);
    serialPanel_ = new SerialSettingsPanel(controller_, modeStack_);
    auto *library = new CommandLibraryPanel(controller_, modeStack_);
    modeStack_->addWidget(serialPanel_);
    modeStack_->addWidget(library);
    col->addWidget(modeStack_, 1);

    connect(modeTabs_, &PivotBar::currentChanged, modeStack_, &QStackedWidget::setCurrentIndex);
    connect(library, &CommandLibraryPanel::openParams, this, [this](int row) { paramsPanel_->openForRow(row); });
    return panel;
}

QWidget *MainView::buildRightSide() {
    auto *side = new QWidget(this);
    auto *col = new QVBoxLayout(side);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(kOuterMargin);

    // data monitor card (the params panel lives at its bottom)
    auto *monitorCard = new Panel(side);
    auto *monitorLayout = new QVBoxLayout(monitorCard);
    monitorLayout->setContentsMargins(1, 1, 1, 1);
    monitor_ = new DataMonitorView(controller_, monitorCard);
    paramsPanel_ = new CommandParamsPanel(controller_, monitorCard);
    monitor_->setBottomWidget(paramsPanel_);
    monitorLayout->addWidget(monitor_);
    col->addWidget(monitorCard, 1);

    connect(serialPanel_, &SerialSettingsPanel::wrapLinesChanged, monitor_, &DataMonitorView::setWrapLines);
    monitor_->setWrapLines(serialPanel_->wrapLines());
    connect(monitor_, &DataMonitorView::saveLogRequested, this, &MainView::openSaveLog);
    connect(monitor_, &DataMonitorView::pausedChanged, this, [this](bool paused) {
        pauseButton_->setChecked(paused);
        pauseAction_->setChecked(paused);
    });

    // send card
    auto *sendCard = new Panel(side);
    auto *sendRow = new QHBoxLayout(sendCard);
    sendRow->setContentsMargins(16, 12, 16, 12);
    sendRow->setSpacing(16);
    input_ = new QLight::PlainTextEdit(sendCard);
    input_->setFont(monoFont());
    input_->setFixedHeight(88);
    input_->setPlainText(controller_->draftText());
    sendRow->addWidget(input_, 1);

    auto *buttons = new QVBoxLayout;
    buttons->setSpacing(4);
    sendButton_ = new QLight::PushButton(QString(), QLight::PushButton::Accent, sendCard);
    clearDraftButton_ = new QLight::PushButton(sendCard);
    historyButton_ = new QLight::PushButton(sendCard);
    for (QLight::PushButton *b : {sendButton_, clearDraftButton_, historyButton_}) {
        b->setFixedSize(120, 28);
        buttons->addWidget(b);
    }
    buttons->addStretch(1);
    sendRow->addLayout(buttons);
    col->addWidget(sendCard);

    connect(input_, &QPlainTextEdit::textChanged, this, [this] {
        const QString text = input_->toPlainText();
        if (text != controller_->draftText()) controller_->setDraftText(text);
    });
    connect(controller_, &AppController::draftTextChanged, this, [this] {
        if (input_->toPlainText() == controller_->draftText()) return;
        input_->setPlainText(controller_->draftText());
        input_->moveCursor(QTextCursor::End);
    });
    connect(sendButton_, &QPushButton::clicked, controller_, &AppController::sendManualText);
    connect(clearDraftButton_, &QPushButton::clicked, controller_, &AppController::clearDraft);
    connect(historyButton_, &QPushButton::clicked, this, &MainView::showHistory);
    return side;
}

QWidget *MainView::buildStatusBar() {
    auto *bar = new QWidget(this);
    bar->setFixedHeight(28);
    auto *row = new QHBoxLayout(bar);
    row->setContentsMargins(kOuterMargin + 2, 0, kOuterMargin, 0);
    row->setSpacing(18);
    statusText_ = new StatusPill(bar);
    statusText_->setFramed(false);
    toastLabel_ = new ThemedLabel(ThemedLabel::Secondary, bar);
    versionLabel_ = new ThemedLabel(QStringLiteral("v") + controller_->appVersion(), ThemedLabel::Secondary, bar);
    row->addWidget(statusText_);
    row->addStretch(1);
    row->addWidget(toastLabel_);
    row->addWidget(versionLabel_);

    toastTimer_ = new QTimer(this);
    toastTimer_->setSingleShot(true);
    toastTimer_->setInterval(kToastMs);
    connect(toastTimer_, &QTimer::timeout, toastLabel_, &QLabel::clear);
    return bar;
}

void MainView::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) retranslateUi();
}

void MainView::paintEvent(QPaintEvent *) {
    QPainter(this).fillRect(rect(), QLight::ThemeManager::instance().palette().windowBg);
}

void MainView::retranslateUi() {
    fileMenu_->setTitle(tr("&File"));
    saveLogAction_->setText(tr("Save log"));
    exitAction_->setText(tr("Exit"));
    editMenu_->setTitle(tr("&Edit"));
    clearAction_->setText(tr("Clear"));
    viewMenu_->setTitle(tr("&View"));
    pauseAction_->setText(tr("Pause scrolling"));
    toolsMenu_->setTitle(tr("&Tools"));
    settingsAction_->setText(tr("Settings"));
    helpMenu_->setTitle(tr("&Help"));
    updateAction_->setText(tr("Check for software update"));
    driversAction_->setText(tr("Driver downloads"));
    aboutAction_->setText(tr("About"));

    saveLogButton_->setText(tr("Save log"));
    pauseButton_->setText(tr("Pause scrolling"));
    clearButton_->setText(tr("Clear"));
    settingsButton_->setText(tr("Settings"));

    driverBar_->setTitle(tr("Serial driver not installed, so devices may not be detected."));
    driverBar_->setActionText(tr("View details"));

    modeTabs_->setTabText(0, tr("Serial"));
    modeTabs_->setTabText(1, tr("Device commands"));

    input_->setPlaceholderText(tr("Type data to send…"));
    sendButton_->setText(tr("Send"));
    clearDraftButton_->setText(tr("Clear"));
    historyButton_->setText(tr("History"));
    updateConnectionState();
}

void MainView::updateConnectionState() {
    const bool open = controller_->portOpen();
    if (open) {
        QString port = controller_->selectedPortName();
        if (port.isEmpty()) port = controller_->portStatusText().section(QLatin1Char(' '), 0, 0);
        statusPill_->setState(true, tr("Connected · %1").arg(port));
        statusText_->setState(true, controller_->portStatusText() + QStringLiteral("  ·  ") + controller_->portSummary());
    } else {
        statusPill_->setState(false, tr("Not connected"));
        statusText_->setState(false, tr("Not connected"));
    }
}

void MainView::updateDriverBar() {
    driverBar_->setVisible(!controller_->ch340DriverInstalled() && !controller_->ch340DriverHintDismissed());
}

void MainView::showToast(const QString &text) {
    toastLabel_->setText(text);
    toastTimer_->start();
}

void MainView::showHistory() {
    if (historyFlyout_) {
        historyFlyout_->close();
        return;
    }
    auto *content = new QWidget;
    content->setFixedWidth(320);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(6);

    auto *head = new QHBoxLayout;
    auto *title = new ThemedLabel(tr("Send history"), ThemedLabel::Primary, content);
    title->setEmphasis(true);
    auto *clear = new QLight::PushButton(tr("Clear history"), QLight::PushButton::Subtle, content);
    head->addWidget(title, 1);
    head->addWidget(clear);
    col->addLayout(head);

    CommandHistoryModel *history = controller_->commandHistoryModel();
    auto *empty = new ThemedLabel(tr("No history yet"), ThemedLabel::Secondary, content);
    empty->setAlignment(Qt::AlignCenter);
    empty->setMinimumHeight(48);
    auto *list = new CommandListView(content);
    list->setReorderEnabled(false);
    list->setModel(history);
    auto *delegate = new CommandItemDelegate(CommandHistoryModel::TextRole, -1, list);
    delegate->setTrailingRole(CommandHistoryModel::TimeRole);
    delegate->setRowHeight(36);
    delegate->setShowMarker(false);
    list->setItemDelegate(delegate);
    list->setFont(monoFont());
    const int rows = history->rowCount();
    list->setFixedHeight(qMin(rows, 7) * 36 + 2);
    empty->setVisible(rows == 0);
    list->setVisible(rows > 0);
    clear->setEnabled(rows > 0);
    col->addWidget(empty);
    col->addWidget(list);

    QLight::Flyout *flyout = QLight::Flyout::open(historyButton_, content, QLight::Flyout::Placement::Top);
    flyout->setPadding(12);
    historyFlyout_ = flyout;
    connect(list, &CommandListView::rowClicked, flyout, [this, flyout](int row) {
        controller_->setDraftText(controller_->commandHistoryModel()->textAt(row));
        flyout->close();
        input_->setFocus();
    });
    connect(clear, &QPushButton::clicked, flyout, [this, flyout] {
        controller_->commandHistoryModel()->clear();
        flyout->close();
    });
}

void MainView::openSaveLog() {
    if (!saveLogDialog_) saveLogDialog_ = new SaveLogDialog(controller_, window());
    saveLogDialog_->openDialog();
}

void MainView::openSettings() {
    if (!settingsDialog_) settingsDialog_ = new SettingsDialog(controller_, window());
    settingsDialog_->openDialog();
}

void MainView::openAbout() {
    if (!aboutDialog_) aboutDialog_ = new AboutDialog(controller_, window());
    aboutDialog_->open();
}

void MainView::openDriverHint() {
    if (!driverHintDialog_) {
        driverHintDialog_ = new DriverHintDialog(controller_, window());
        connect(driverHintDialog_, &DriverHintDialog::downloadRequested, this, &MainView::openDriverList);
    }
    driverHintDialog_->open();
}

void MainView::openDriverList() {
    if (!driverListDialog_) driverListDialog_ = new DriverListDialog(controller_, window());
    driverListDialog_->openDialog();
}

void MainView::openSoftwareUpdate() {
    controller_->checkForAppUpdate();
    if (!softwareUpdateDialog_) softwareUpdateDialog_ = new SoftwareUpdateDialog(controller_, window());
    softwareUpdateDialog_->open();
}

}  // namespace ui
