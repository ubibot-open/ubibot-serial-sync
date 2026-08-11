import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

ApplicationWindow {
    id: window
    width: 1200
    height: 800
    visible: true
    title: qsTr("UbiBot Serial Assistant")
    color: Theme.background

    Connections {
        target: AppController
        function onWizardFinished() { modeBar.currentIndex = 0 }
        function onPortOpenFailed(error) { portErrorDialog.text = error; portErrorDialog.open() }
        function onStatusMessage(text) { statusToast.show(text) }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem { text: qsTr("Connection wizard"); onTriggered: wizardDialog.open() }
            MenuItem { text: qsTr("Save log"); onTriggered: saveLogDialog.open() }
            MenuSeparator {}
            MenuItem { text: qsTr("Exit"); onTriggered: Qt.quit() }
        }
        Menu {
            title: qsTr("&Edit")
            MenuItem { text: qsTr("Clear"); onTriggered: AppController.logModel.clear() }
        }
        Menu {
            title: qsTr("&View")
            MenuItem {
                text: qsTr("Pause scrolling")
                checkable: true
                checked: monitor.paused
                onTriggered: monitor.paused = checked
            }
        }
        Menu {
            title: qsTr("&Tools")
            MenuItem { text: qsTr("Settings"); onTriggered: settingsDialog.open() }
        }
        Menu {
            title: qsTr("&Help")
            MenuItem { text: qsTr("Settings"); onTriggered: settingsDialog.open() }
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 6
            spacing: 2

            ToolButton { icon.source: "qrc:/icons/wizard.svg"; text: qsTr("Connection wizard"); onClicked: wizardDialog.open() }
            ToolButton { icon.source: "qrc:/icons/save.svg"; text: qsTr("Save log"); onClicked: saveLogDialog.open() }
            ToolSeparator {}
            ToolButton { icon.source: "qrc:/icons/send.svg"; text: qsTr("Send"); onClicked: AppController.sendManualText() }
            ToolButton {
                icon.source: "qrc:/icons/pause.svg"
                text: qsTr("Pause scrolling")
                checkable: true
                checked: monitor.paused
                onToggled: monitor.paused = checked
            }
            ToolButton { icon.source: "qrc:/icons/clear.svg"; text: qsTr("Clear"); onClicked: AppController.logModel.clear() }
            ToolSeparator {}
            ToolButton { icon.source: "qrc:/icons/settings.svg"; text: qsTr("Settings"); onClicked: settingsDialog.open() }
            Item { Layout.fillWidth: true }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- mode selector + device badge + port toggle ----------------------
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 52
            color: "transparent"
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 10

                TabBar {
                    id: modeBar
                    TabButton { text: qsTr("Device commands") }
                    TabButton { text: qsTr("Serial") }
                    TabButton { text: qsTr("Remote support") }
                }

                Item { Layout.fillWidth: true }

                ColumnLayout {
                    spacing: 0
                    Label {
                        text: AppController.currentModelId
                        font.family: Theme.monoFont
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        text: qsTr("Current device")
                        font.pixelSize: 10
                        color: Theme.textMuted
                        Layout.alignment: Qt.AlignRight
                    }
                }

                Button {
                    text: AppController.portOpen ? qsTr("Close port") : qsTr("Open port")
                    highlighted: true
                    Layout.preferredWidth: 120
                    onClicked: {
                        if (AppController.portOpen) {
                            AppController.closePort()
                        } else {
                            AppController.openPort(serialPanel.selectedPort, serialPanel.selectedBaud,
                                                    serialPanel.selectedDataBits, serialPanel.selectedParity,
                                                    serialPanel.selectedStopBits, serialPanel.selectedFlowControl)
                        }
                    }
                }
            }
        }

        // --- body: left mode-specific panel + right data monitor -------------
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: 330
                Layout.fillHeight: true
                color: "transparent"
                Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Theme.divider }

                StackLayout {
                    anchors.fill: parent
                    currentIndex: modeBar.currentIndex

                    CommandLibraryPanel {
                        onOpenParams: (row) => paramsPanel.openForRow(row)
                    }
                    SerialSettingsPanel { id: serialPanel }
                    RemoteAssistPanel { }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                DataMonitorView {
                    id: monitor
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapLines: serialPanel.wrapLines
                }

                CommandParamsPanel {
                    id: paramsPanel
                    Layout.fillWidth: true
                    Layout.margins: visible ? 14 : 0
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: 12
                    spacing: 10

                    TextArea {
                        id: inputField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 74
                        font.family: Theme.monoFont
                        placeholderText: qsTr("Type data to send…")
                        text: AppController.draftText
                        onTextChanged: if (text !== AppController.draftText) AppController.draftText = text

                        background: Rectangle { border.color: Theme.divider; border.width: 1 }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 100
                        Button { text: qsTr("Send"); highlighted: true; Layout.fillWidth: true; onClicked: AppController.sendManualText() }
                        Button { text: qsTr("Clear"); Layout.fillWidth: true; onClicked: AppController.clearDraft() }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    color: "transparent"
                    Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: Theme.divider }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 16

                        Label {
                            text: AppController.portStatusText
                            color: AppController.portOpen ? Theme.accent700 : Theme.error
                            font.family: Theme.monoFont
                            font.pixelSize: 11
                        }
                        Label { text: AppController.portSummary; font.family: Theme.monoFont; font.pixelSize: 11 }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: statusToast.visibleText
                            font.pixelSize: 11
                            color: Theme.textMuted
                        }
                    }
                }
            }
        }
    }

    // --- dialogs ---------------------------------------------------------
    ConnectionWizardDialog { id: wizardDialog }
    SaveLogDialog { id: saveLogDialog }
    SettingsAboutDialog { id: settingsDialog }

    Dialog {
        id: portErrorDialog
        property alias text: errorLabel.text
        title: qsTr("Failed to open port")
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok
        Label { id: errorLabel; width: 320; wrapMode: Text.WordWrap }
    }

    // Transient status-bar message (e.g. "Log saved to ..."), matching the
    // old QStatusBar::showMessage(text, 5000) behavior.
    QtObject {
        id: statusToast
        property string visibleText: ""
        function show(text) {
            visibleText = text
            hideTimer.restart()
        }
        property Timer hideTimer: Timer {
            interval: 5000
            onTriggered: statusToast.visibleText = ""
        }
    }
}
