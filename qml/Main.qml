// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Window
import UbiBot

ApplicationWindow {
    id: window

    readonly property int shadowMargin: 18
    readonly property bool shadowActive: visibility !== Window.Maximized && visibility !== Window.FullScreen
    readonly property int activeMargin: shadowActive ? shadowMargin : 0

    width: 1320 + shadowMargin * 2
    height: 860 + shadowMargin * 2
    minimumWidth: 860 + shadowMargin * 2
    minimumHeight: 600 + shadowMargin * 2
    visible: true
    title: qsTr("UbiBot Serial Assistant")
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint
    palette: Theme.palette
    font.family: Theme.baseFontFamily
    font.pixelSize: Theme.baseFontSize

    Connections {
        target: AppController
        function onPortOpenFailed(error) { portErrorDialog.text = error; portErrorDialog.open() }
        function onStatusMessage(text) { statusToast.show(text) }
    }

    component CompactToolButton: ToolButton {
        display: AbstractButton.IconOnly
        implicitWidth: 32
        implicitHeight: 32
        padding: 4
        icon.color: Theme.text
        ToolTip.visible: hovered
        ToolTip.delay: 400
    }

    component TitleBar: Rectangle {
        id: titleBar
        implicitHeight: 32
        color: Theme.surface
        MouseArea {
            anchors.fill: parent
            onPressed: (mouse) => { if (mouse.button === Qt.LeftButton) window.startSystemMove() }
            onDoubleClicked: window.visibility = (window.visibility === Window.Maximized) ? Window.Windowed : Window.Maximized
        }

        RowLayout {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 12
            spacing: 10
            Image { source: "qrc:/icons/app.png"; sourceSize: Qt.size(20, 20); width: 16; height: 16 }
            Label { text: window.title; color: Theme.text; font.pixelSize: Theme.baseFontSize }
            Label {
                text: "v" + AppController.appVersion
                color: Theme.textMuted
                font.pixelSize: Theme.baseFontSize - 1
            }
        }

        Row {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            spacing: 0

            CaptionButton {
                onClicked: window.showMinimized()
                Rectangle { anchors.centerIn: parent; width: 10; height: 1; color: Theme.text }
            }
            CaptionButton {
                onClicked: window.visibility = (window.visibility === Window.Maximized) ? Window.Windowed : Window.Maximized
                Rectangle {
                    visible: window.visibility !== Window.Maximized
                    anchors.centerIn: parent
                    width: 10; height: 10
                    color: "transparent"
                    border.color: Theme.text
                    border.width: 1
                }
                Item {
                    visible: window.visibility === Window.Maximized
                    anchors.centerIn: parent
                    width: 12; height: 12
                    Rectangle { x: 3; y: 0; width: 8; height: 8; color: "transparent"; border.color: Theme.text; border.width: 1 }
                    Rectangle { x: 0; y: 3; width: 8; height: 8; color: Theme.surface; border.color: Theme.text; border.width: 1 }
                }
            }
            CaptionButton {
                danger: true
                onClicked: window.close()
                Item {
                    anchors.centerIn: parent
                    width: 12; height: 12
                    Rectangle { anchors.centerIn: parent; width: 14; height: 1.4; color: Theme.text; rotation: 45 }
                    Rectangle { anchors.centerIn: parent; width: 14; height: 1.4; color: Theme.text; rotation: -45 }
                }
            }
        }
    }
    component CaptionButton: Rectangle {
        id: capBtn
        property bool danger: false
        signal clicked()
        implicitWidth: 46
        implicitHeight: 32
        color: hoverArea.containsMouse ? (danger ? "#e81123" : Theme.divider) : "transparent"
        MouseArea {
            id: hoverArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: capBtn.clicked()
        }
    }

    // The MultiEffect shadow below renders this rect even while it is
    // invisible; when the shadow is off (maximized/fullscreen) the rect
    // must paint itself, otherwise the transparent window shows the
    // desktop through every unpainted region (e.g. the left panel).
    Rectangle {
        id: frameBackdrop
        anchors.fill: frame
        visible: !window.shadowActive
        color: Theme.background
    }

    Item {
        id: frame
        anchors.fill: parent
        anchors.margins: window.activeMargin

        ColumnLayout {
            id: headerColumn
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 0

            TitleBar { Layout.fillWidth: true }

            MenuBar {
                Layout.fillWidth: true
                Menu {
                    title: qsTr("&File")
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
                    // Commented out (not removed) -- the self-update feature
                    // needs ubibot-appcenter's Software/Version backend to
                    // actually have this app registered (see
                    // docs/app-self-update.md#2) plus a real Version row
                    // uploaded for it, and neither has been deployed yet.
                    // Re-enable by uncommenting once that's live; nothing
                    // else needs to change -- AppController/
                    // SoftwareUpdateClient/SoftwareUpdateDialog.qml are all
                    // still fully wired up, this menu item was the only
                    // user-facing entry point into any of it.
                    // Named "software update" (not just "Check for
                    // updates") specifically to not collide with the
                    // unrelated "Check for updates" button already living
                    // in Settings & About's "Command library" section
                    // (device command data, not the app itself -- see
                    // SettingsAboutDialog.qml).
                    // MenuItem {
                    //     text: qsTr("Check for software update")
                    //     onTriggered: {
                    //         AppController.checkForAppUpdate();
                    //         softwareUpdateDialog.open();
                    //     }
                    // }
                    // MenuSeparator {}
                    // Always available regardless of whether the driver-
                    // hint banner (below the toolbar) has ever shown or
                    // been dismissed -- per user feedback, someone who
                    // clicked the banner's "✕" shouldn't lose access to
                    // this altogether. Opens the same DriverListDialog the
                    // banner's own hint dialog's "Download driver" button
                    // does -- see docs/ch340-driver-hint.md.
                    MenuItem { text: qsTr("Driver downloads"); onTriggered: driverListDialog.open() }
                    MenuSeparator {}
                    MenuItem { text: qsTr("About"); onTriggered: aboutDialog.open() }
                }
            }

            ToolBar {
                Layout.fillWidth: true
                implicitHeight: 52

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 12
                    spacing: 4

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
                }
            }

            Rectangle {
                id: driverHintBanner
                Layout.fillWidth: true
                visible: !AppController.ch340DriverInstalled && !AppController.ch340DriverHintDismissed
                implicitHeight: visible ? driverHintRow.implicitHeight + 12 : 0
                color: Theme.surface

                Rectangle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; width: 3; color: Theme.error }
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }

                MouseArea {
                    anchors.fill: parent
                    anchors.rightMargin: 32
                    cursorShape: Qt.PointingHandCursor
                    onClicked: driverHintDialog.open()
                }

                RowLayout {
                    id: driverHintRow
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 8
                    spacing: 8

                    Label {
                        Layout.fillWidth: true
                        color: Theme.text
                        text: qsTr("Serial driver not installed — click for details")
                    }
                    ToolButton {
                        text: "✕"
                        flat: true
                        ToolTip.text: qsTr("Dismiss")
                        ToolTip.visible: hovered
                        onClicked: AppController.ch340DriverHintDismissed = true
                    }
                }
            }
        } // headerColumn

        ColumnLayout {
            anchors.top: headerColumn.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            spacing: 0

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
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 61

                        Row {
                            id: modeBar
                            anchors.centerIn: parent
                            width: parent.width - 28
                            property int currentIndex: 0
                            readonly property var labels: [qsTr("Serial"), qsTr("Device commands")]

                            Repeater {
                                model: modeBar.labels
                                delegate: Rectangle {
                                    id: segment
                                    required property string modelData
                                    required property int index
                                    width: Math.floor(modeBar.width / 2)
                                    height: 36
                                    clip: true
                                    color: modeBar.currentIndex === index ? Theme.accent : "transparent"
                                    border.color: Theme.divider
                                    border.width: 1

                                    Label {
                                        anchors.centerIn: parent
                                        width: Math.min(implicitWidth, parent.width - 12)
                                        horizontalAlignment: Text.AlignHCenter
                                        elide: Text.ElideRight
                                        text: segment.modelData
                                        font.pixelSize: Theme.baseFontSize + 1
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
                            SerialSettingsPanel { id: serialPanel }
                            CommandLibraryPanel {
                                onOpenParams: (row) => paramsPanel.openForRow(row)
                            }
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
                    Layout.margins: visible ? 16 : 0
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: 14
                    spacing: 12

                    ScrollView {
                        id: inputScroll
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.max(74, Math.min(200, inputField.contentHeight + 20))
                        clip: true

                        TextArea {
                            id: inputField
                            font.family: Theme.monoFont
                            placeholderText: qsTr("Type data to send…")
                            placeholderTextColor: Theme.textMuted
                            Component.onCompleted: text = AppController.draftText
                            onTextChanged: if (text !== AppController.draftText) AppController.draftText = text
                            Connections {
                                target: AppController
                                function onDraftTextChanged() {
                                    if (inputField.text !== AppController.draftText) inputField.text = AppController.draftText
                                }
                            }
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
                    font.family: Theme.baseFontFamily
                    font.pixelSize: Theme.baseFontSize
                    background: DialogCard {}

                    contentItem: ColumnLayout {
                        spacing: 0

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.margins: 10
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
                            font.pixelSize: Theme.baseFontSize
                            Layout.margins: 16
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
                                implicitHeight: historyRow.implicitHeight + 14
                                onClicked: {
                                    AppController.draftText = historyDelegate.commandText
                                    historyPopup.close()
                                }

                                RowLayout {
                                    id: historyRow
                                    anchors.fill: parent
                                    anchors.margins: 9
                                    Label {
                                        text: historyDelegate.commandText
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                        font.family: Theme.monoFont
                                        font.pixelSize: Theme.baseFontSize
                                    }
                                    Label {
                                        text: historyDelegate.timeText
                                        visible: historyDelegate.timeText.length > 0
                                        color: Theme.textMuted
                                        font.pixelSize: Theme.baseFontSize - 2
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
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 18

                        Label {
                            text: AppController.portStatusText
                            color: AppController.portOpen ? Theme.accent700 : Theme.error
                            font.family: Theme.monoFont
                            font.pixelSize: Theme.baseFontSize - 1
                        }
                        Label { text: AppController.portSummary; font.family: Theme.monoFont; font.pixelSize: Theme.baseFontSize - 1 }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: statusToast.visibleText
                            font.pixelSize: Theme.baseFontSize - 1
                            color: Theme.textMuted
                        }
                    }
                }
            }
        }
        } // closes the body ColumnLayout (re-anchored below headerColumn, see its own comment above)
        MouseArea {
            visible: window.shadowActive
            height: window.resizeGrip
            anchors { top: parent.top; left: parent.left; right: parent.right }
            cursorShape: Qt.SizeVerCursor
            onPressed: window.startSystemResize(Qt.TopEdge)
        }
        MouseArea {
            visible: window.shadowActive
            height: window.resizeGrip
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            cursorShape: Qt.SizeVerCursor
            onPressed: window.startSystemResize(Qt.BottomEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            cursorShape: Qt.SizeHorCursor
            onPressed: window.startSystemResize(Qt.LeftEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
            cursorShape: Qt.SizeHorCursor
            onPressed: window.startSystemResize(Qt.RightEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip * 2
            height: window.resizeGrip * 2
            anchors { top: parent.top; left: parent.left }
            cursorShape: Qt.SizeFDiagCursor
            onPressed: window.startSystemResize(Qt.TopEdge | Qt.LeftEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip * 2
            height: window.resizeGrip * 2
            anchors { top: parent.top; right: parent.right }
            cursorShape: Qt.SizeBDiagCursor
            onPressed: window.startSystemResize(Qt.TopEdge | Qt.RightEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip * 2
            height: window.resizeGrip * 2
            anchors { bottom: parent.bottom; left: parent.left }
            cursorShape: Qt.SizeBDiagCursor
            onPressed: window.startSystemResize(Qt.BottomEdge | Qt.LeftEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip * 2
            height: window.resizeGrip * 2
            anchors { bottom: parent.bottom; right: parent.right }
            cursorShape: Qt.SizeFDiagCursor
            onPressed: window.startSystemResize(Qt.BottomEdge | Qt.RightEdge)
        }
        Rectangle {
            anchors.fill: parent
            visible: window.shadowActive
            color: "transparent"
            border.color: Theme.dialogBorder
            border.width: 1
        }
    } // frame
    readonly property int resizeGrip: 4
    MultiEffect {
        anchors.fill: frameBackdrop
        source: frameBackdrop
        visible: window.shadowActive
        autoPaddingEnabled: true
        shadowEnabled: true
        shadowColor: "black"
        shadowOpacity: 0.35
        shadowBlur: 0.6
        shadowHorizontalOffset: 0
        shadowVerticalOffset: 3
        z: -1
    }

    // --- dialogs ---------------------------------------------------------
    SaveLogDialog { id: saveLogDialog }
    SettingsAboutDialog { id: settingsDialog }
    AboutDialog { id: aboutDialog }
    SoftwareUpdateDialog { id: softwareUpdateDialog }
    DriverListDialog { id: driverListDialog }

    Dialog {
        id: portErrorDialog
        property alias text: errorLabel.text
        title: qsTr("Failed to open port")
        modal: true
        anchors.centerIn: Overlay.overlay
        palette: Theme.palette
        font.family: Theme.baseFontFamily
        font.pixelSize: Theme.baseFontSize
        background: DialogCard {}
        Overlay.modal: ModalDim {}
        header: Label {
            text: portErrorDialog.title
            font.bold: true
            padding: 14
            color: Theme.text
            background: Rectangle { color: Theme.surface }
        }
        footer: DialogButtonBox {
            palette: Theme.palette
            background: Rectangle { color: Theme.surface }
            Button {
                text: qsTr("OK")
                palette: Theme.palette
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
        }
        Label { id: errorLabel; width: 320; wrapMode: Text.WordWrap; color: Theme.text }
    }
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
