#pragma once

#include "core/device_library.h"
#include "core/log_manager.h"
#include "core/serial_manager.h"
#include "core/settings_store.h"

#include <QMainWindow>

class QAction;
class QButtonGroup;
class QLabel;
class QMenu;
class QPushButton;
class QStackedWidget;
class QTextEdit;
class QTimer;

class SerialSettingsPanel;
class CommandLibraryPanel;
class RemoteAssistPanel;
class CommandParamsPanel;
class DataMonitorView;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void buildUi();
    void buildMenus();
    void buildToolbar();
    void buildStatusBar();
    void wireSignals();
    void retranslateUi();

    void openWizardDialog();
    void openSaveLogDialog();
    void openSettingsDialog();
    void togglePort();
    void sendManualInput();
    void updateConnectionUi();

    QByteArray composeAsciiPayload(const QString &text) const;
    QByteArray composeHexPayload(const QString &text) const;
    void sendLiteral(const QString &text);

    // --- core, non-UI state --------------------------------------------
    DeviceLibrary library_;
    SettingsStore settings_;
    SerialManager serial_;
    LogManager logManager_;
    QTimer *repeatTimer_;

    // --- central widgets --------------------------------------------------
    QStackedWidget *modeStack_;
    QButtonGroup *modeButtons_;
    QPushButton *cmdModeButton_;
    QPushButton *serialModeButton_;
    QPushButton *remoteModeButton_;

    SerialSettingsPanel *serialPanel_;
    CommandLibraryPanel *commandPanel_;
    RemoteAssistPanel *remotePanel_;
    DataMonitorView *monitorView_;
    CommandParamsPanel *paramsPanel_;
    QTextEdit *inputEdit_;
    QPushButton *sendButton_;
    QPushButton *clearInputButton_;

    QLabel *modelBadgeCaption_;
    QLabel *modelBadgeValue_;
    QPushButton *portToggleButton_;

    // --- menu & toolbar -----------------------------------------------
    QMenu *fileMenu_;
    QMenu *editMenu_;
    QMenu *viewMenu_;
    QMenu *toolsMenu_;
    QMenu *helpMenu_;

    QAction *wizardAction_;
    QAction *saveLogAction_;
    QAction *sendAction_;
    QAction *pauseAction_;
    QAction *clearAction_;
    QAction *settingsAction_;
    QAction *exitAction_;
    QAction *aboutQtAction_;

    // --- status bar -----------------------------------------------------
    QLabel *connectionStatusLabel_;
    QLabel *portParamsLabel_;
    QLabel *byteCountersLabel_;
};
