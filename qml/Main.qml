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

    // Fusion (set in main.cpp) is one of the few built-in styles that
    // actually honors palette roles, so this is a cheap way to pull the
    // whole app's default control coloring toward the design's light/blue
    // theme without hand-skinning every control type. Popups/Dialogs don't
    // reliably inherit this from the ApplicationWindow, though -- each one
    // sets `palette: Theme.palette` itself too (see Theme.qml).
    palette: Theme.palette

    Connections {
        target: AppController
        function onWizardFinished() { modeBar.currentIndex = 0 }
        function onPortOpenFailed(error) { portErrorDialog.text = error; portErrorDialog.open() }
        function onStatusMessage(text) { statusToast.show(text) }
    }

    // Fusion's default ToolButton padding is generous (meant for
    // text+icon buttons); the design's toolbar is a tight row of plain
    // 32x32 icon squares, so the toolbar below uses this instead of relying
    // on the style default. Inline components must live at the document's
    // top level, hence declaring it here rather than inside `header:`.
    component CompactToolButton: ToolButton {
        display: AbstractButton.IconOnly
        implicitWidth: 32
        implicitHeight: 32
        padding: 4
        ToolTip.visible: hovered
        ToolTip.delay: 400
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

    // Icon-only toolbar (matches the original design's compact 32x32 icon
    // buttons) with the current-device badge and the open/close-port button
    // at the trailing end, all in the one row the design puts them in.
    header: ToolBar {
        // Design's toolbar row is a fixed 52px band (32px icon buttons plus
        // 10px of breathing room top and bottom) -- pin it explicitly since
        // Fusion's default ToolBar padding varies by platform.
        implicitHeight: 52

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 10
            spacing: 0

            CompactToolButton {
                icon.source: "qrc:/icons/wizard.svg"
                ToolTip.text: qsTr("Connection wizard")
                onClicked: wizardDialog.open()
            }
            CompactToolButton {
                icon.source: "qrc:/icons/save.svg"
                ToolTip.text: qsTr("Save log")
                onClicked: saveLogDialog.open()
            }
            ToolSeparator { Layout.preferredHeight: 22; Layout.leftMargin: 4; Layout.rightMargin: 4 }
            CompactToolButton {
                icon.source: "qrc:/icons/send.svg"
                ToolTip.text: qsTr("Send")
                onClicked: AppController.sendManualText()
            }
            CompactToolButton {
                icon.source: "qrc:/icons/pause.svg"
                checkable: true
                checked: monitor.paused
                ToolTip.text: qsTr("Pause scrolling")
                onToggled: monitor.paused = checked
            }
            CompactToolButton {
                icon.source: "qrc:/icons/clear.svg"
                ToolTip.text: qsTr("Clear")
                onClicked: AppController.logModel.clear()
            }
            ToolSeparator { Layout.preferredHeight: 22; Layout.leftMargin: 4; Layout.rightMargin: 4 }
            CompactToolButton {
                icon.source: "qrc:/icons/settings.svg"
                ToolTip.text: qsTr("Settings")
                onClicked: settingsDialog.open()
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
                Layout.preferredWidth: 112
                Layout.leftMargin: 10
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

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- body: left mode-specific panel + right data monitor -------------
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            Rectangle {
                Layout.preferredWidth: 330
                Layout.fillWidth: false
                Layout.fillHeight: true
                color: "transparent"
                border.color: Theme.divider
                border.width: 1
                ColumnLayout {
                    anchors.fill: parent
                    // The three-way mode switch lives inside the sidebar itself
                    // (full-width, equal thirds), not in the top toolbar --
                    // matching the original design's segmented control rather
                    // than competing for space with the toolbar's icons and
                    // device badge. Hand-rolled with Row + Repeater instead of
                    // TabBar/TabButton: TabBar's internal item positioner
                    // doesn't reliably honor per-tab width overrides, which is
                    // exactly what caused the earlier label-truncation bug.
                    // Design insets the segmented control 14px from the sidebar's
                    // edges inside a 61px-tall band, rather than butting it up
                    // against the sidebar's own edges.
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 61

                        Row {
                            id: modeBar
                            anchors.centerIn: parent
                            width: parent.width - 28
                            property int currentIndex: 0
                            readonly property var labels: [qsTr("Device commands"), qsTr("Serial")]

                            Repeater {
                                model: modeBar.labels
                                delegate: Rectangle {
                                    id: segment
                                    required property string modelData
                                    required property int index
                                    width: Math.floor(modeBar.width / 2)
                                    height: 36
                                    color: modeBar.currentIndex === index ? Theme.accent : "transparent"
                                    border.color: Theme.divider
                                    border.width: 1

                                    Label {
                                        anchors.centerIn: parent
                                        text: segment.modelData
                                        font.pixelSize: 13
                                        color: modeBar.currentIndex === index ? Theme.accentForeground : Theme.text
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: modeBar.currentIndex = segment.index
                                    }
                                }
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.divider }

                    Rectangle {
                        Layout.fillWidth: true
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
                    onSaveLogRequested: saveLogDialog.open()
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

                    ScrollView {
                        id: inputScroll
                        Layout.fillWidth: true
                        // Starts at the original fixed 74px. Multi-line
                        // content beyond that grows the box (so short input
                        // still looks the same as before) up to 200px, past
                        // which it stops growing and a scrollbar takes over
                        // instead -- otherwise overflowing lines were simply
                        // invisible with no way to reach them.
                        Layout.preferredHeight: Math.max(74, Math.min(200, inputField.contentHeight + 20))
                        clip: true

                        TextArea {
                            id: inputField
                            font.family: Theme.monoFont
                            placeholderText: qsTr("Type data to send…")
                            // Deliberately not a `text: AppController.draftText`
                            // binding -- QML drops a property's binding as soon
                            // as anything assigns to it directly, and every
                            // keystroke does exactly that to `text`. After the
                            // user's first keystroke this binding would be gone
                            // for good, so external changes to draftText (Clear,
                            // double-clicking a history entry below) would stop
                            // reaching the box. The Connections handler below
                            // does that sync imperatively instead, which keeps
                            // working no matter how the user has edited the text.
                            Component.onCompleted: text = AppController.draftText
                            onTextChanged: if (text !== AppController.draftText) AppController.draftText = text
                            Connections {
                                target: AppController
                                function onDraftTextChanged() {
                                    if (inputField.text !== AppController.draftText) inputField.text = AppController.draftText
                                }
                            }

                            // Was missing an explicit `color:` -- a bare
                            // Rectangle defaults to white, which happened to
                            // look right against the old fixed light theme
                            // but stayed glaring white against a dark one.
                            background: Rectangle { color: Theme.surface; border.color: Theme.divider; border.width: 1 }
                        }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 116
                        Layout.fillWidth: false
                        Button { text: qsTr("Send"); highlighted: true; Layout.fillWidth: true; onClicked: AppController.sendManualText() }
                        Button { text: qsTr("Clear"); Layout.fillWidth: true; onClicked: AppController.clearDraft() }
                        Button {
                            id: historyButton
                            text: qsTr("History")
                            Layout.fillWidth: true
                            onClicked: historyPopup.visible ? historyPopup.close() : historyPopup.open()
                        }
                    }
                }

                // Recently-sent manual text, newest first. Opens upward from
                // the History button since this row sits near the bottom of
                // the window -- opening downward would mostly land off-screen.
                Popup {
                    id: historyPopup
                    parent: historyButton
                    x: historyButton.width - width
                    y: -height - 6
                    width: 320
                    padding: 0
                    modal: false
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                    palette: Theme.palette

                    background: Rectangle { color: Theme.background; border.color: Theme.divider; border.width: 1 }

                    contentItem: ColumnLayout {
                        spacing: 0

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.margins: 8
                            Label { text: qsTr("Send history"); font.bold: true; Layout.fillWidth: true }
                            ToolButton {
                                text: qsTr("Clear history")
                                flat: true
                                enabled: historyList.count > 0
                                onClicked: AppController.commandHistoryModel.clear()
                            }
                        }
                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.divider }

                        Label {
                            visible: historyList.count === 0
                            text: qsTr("No history yet")
                            color: Theme.textMuted
                            font.pixelSize: 12
                            Layout.margins: 14
                            Layout.alignment: Qt.AlignHCenter
                        }

                        ListView {
                            id: historyList
                            visible: count > 0
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.min(contentHeight, 260)
                            clip: true
                            model: AppController.commandHistoryModel
                            delegate: ItemDelegate {
                                id: historyDelegate
                                required property string commandText
                                required property string timeText
                                width: ListView.view.width
                                // Same reasoning as the wizard's list delegates:
                                // ItemDelegate's own implicit height ignores
                                // whatever is stuffed into it below, so size off
                                // the row's implicit height instead.
                                implicitHeight: historyRow.implicitHeight + 14

                                // Double-click (not single-click) loads the
                                // entry -- a single click while just browsing
                                // history shouldn't clobber whatever the user
                                // is currently typing.
                                onDoubleClicked: {
                                    AppController.draftText = historyDelegate.commandText
                                    historyPopup.close()
                                }

                                RowLayout {
                                    id: historyRow
                                    anchors.fill: parent
                                    anchors.margins: 7
                                    Label {
                                        text: historyDelegate.commandText
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                        font.family: Theme.monoFont
                                        font.pixelSize: 12
                                    }
                                    Label {
                                        text: historyDelegate.timeText
                                        visible: historyDelegate.timeText.length > 0
                                        color: Theme.textMuted
                                        font.pixelSize: 10
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
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
        palette: Theme.palette
        background: Rectangle { color: Theme.background; border.color: Theme.divider; border.width: 1 }
        header: Label {
            text: portErrorDialog.title
            font.bold: true
            padding: 12
            color: Theme.text
            background: Rectangle { color: Theme.background }
        }
        Label { id: errorLabel; width: 320; wrapMode: Text.WordWrap; color: Theme.text }
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
