// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <QPointer>
#include <QWidget>

class AppController;
class QAction;
class QMenu;
class QStackedWidget;
class QTimer;

namespace QLight {
class Flyout;
class InfoBar;
class MenuBar;
class PlainTextEdit;
class PushButton;
class ToolButton;
}  // namespace QLight

namespace ui {

class AboutDialog;
class CommandParamsPanel;
class DataMonitorView;
class DriverHintDialog;
class DriverListDialog;
class PivotBar;
class SaveLogDialog;
class SerialSettingsPanel;
class SettingsDialog;
class SoftwareUpdateDialog;
class StatusPill;
class ThemedLabel;

// Everything below the title bar: menu bar, command bar, driver hint bar,
// left panel (Serial / Device commands), data monitor, send box, status bar.
class MainView : public QWidget {
    Q_OBJECT
public:
    explicit MainView(AppController *controller, QWidget *parent = nullptr);

    // The menu bar is created here but may be re-parented by the window
    // (e.g. into a native macOS menu bar). Owned by this view otherwise.
    QLight::MenuBar *menuBar() const { return menuBar_; }

protected:
    void changeEvent(QEvent *e) override;
    void paintEvent(QPaintEvent *) override;

private:
    QWidget *buildCommandBar();
    QWidget *buildLeftPanel();
    QWidget *buildRightSide();
    QWidget *buildStatusBar();
    void buildMenus();
    void retranslateUi();
    void updateConnectionState();
    void updateDriverBar();
    void showHistory();
    void showToast(const QString &text);

    void openSaveLog();
    void openSettings();
    void openAbout();
    void openDriverHint();
    void openDriverList();
    void openSoftwareUpdate();

    AppController *controller_;

    QLight::MenuBar *menuBar_;
    QMenu *fileMenu_;
    QMenu *editMenu_;
    QMenu *viewMenu_;
    QMenu *toolsMenu_;
    QMenu *helpMenu_;
    QAction *saveLogAction_;
    QAction *exitAction_;
    QAction *clearAction_;
    QAction *pauseAction_;
    QAction *settingsAction_;
    QAction *driversAction_;
    QAction *aboutAction_;

    QLight::ToolButton *saveLogButton_;
    QLight::ToolButton *pauseButton_;
    QLight::ToolButton *clearButton_;
    QLight::ToolButton *settingsButton_;
    StatusPill *statusPill_;

    QLight::InfoBar *driverBar_;

    PivotBar *modeTabs_;
    QStackedWidget *modeStack_;
    SerialSettingsPanel *serialPanel_;

    DataMonitorView *monitor_;
    CommandParamsPanel *paramsPanel_;
    QLight::PlainTextEdit *input_;
    QLight::PushButton *sendButton_;
    QLight::PushButton *clearDraftButton_;
    QLight::PushButton *historyButton_;
    QPointer<QLight::Flyout> historyFlyout_;

    StatusPill *statusText_;
    ThemedLabel *toastLabel_;
    ThemedLabel *versionLabel_;
    QTimer *toastTimer_;

    SaveLogDialog *saveLogDialog_ = nullptr;
    SettingsDialog *settingsDialog_ = nullptr;
    AboutDialog *aboutDialog_ = nullptr;
    DriverHintDialog *driverHintDialog_ = nullptr;
    DriverListDialog *driverListDialog_ = nullptr;
    SoftwareUpdateDialog *softwareUpdateDialog_ = nullptr;
};

}  // namespace ui
