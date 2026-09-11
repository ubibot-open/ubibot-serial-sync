// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts
import UbiBot

// Right-hand "data monitor" pane: a scrolling, uncolored view of every
// TX/RX/SYS/ERR line, with a small header showing line count and byte
// counters. Pauses purely at the view level -- AppController.logModel keeps
// recording regardless, so unpausing shows everything that happened while
// paused.

Item {
    id: root
    property bool paused: false
    property bool wrapLines: true
    signal saveLogRequested()

    function stripLineColumns(text) {
        const prefix = /^(?:\d{2}:\d{2}:\d{2}  )?(?:(?:TX|RX)   |(?:SYS|ERR)  )/gm
        return text.replace(prefix, "")
    }
    function formatCount(n) {
        return String(n)
    }
    function formatBytes(n) {
        if (n < 1024) return n + " B"
        if (n < 1024 * 1024) return (n / 1024).toFixed(1) + " KB"
        if (n < 1024 * 1024 * 1024) return (n / (1024 * 1024)).toFixed(1) + " MB"
        return (n / (1024 * 1024 * 1024)).toFixed(1) + " GB"
    }

    // Copies the active selection (with its timestamp/tag columns
    // stripped) if there is one, otherwise the entire scrollback verbatim
    // -- a full-log copy keeps that context since it reads as an export,
    // not a value someone's about to paste elsewhere.
    function copyLog() {
        if (contentEdit.selectedText.length > 0) {
            clipboardHelper.text = root.stripLineColumns(contentEdit.selectedText)
            clipboardHelper.selectAll()
            clipboardHelper.copy()
            return
        }
        contentEdit.selectAll()
        contentEdit.copy()
        contentEdit.deselect()
    }

    TextEdit { id: clipboardHelper; visible: false }
    component DarkMenuItem: MenuItem {
        id: darkItem
        implicitWidth: 200
        implicitHeight: 32
        icon.width: 15
        icon.height: 15
        icon.color: darkItem.enabled ? Theme.consoleText : Theme.consoleMuted
        contentItem: RowLayout {
            spacing: 12
            IconImage {
                source: darkItem.icon.source
                color: darkItem.icon.color
                Layout.preferredWidth: darkItem.icon.width
                Layout.preferredHeight: darkItem.icon.height
                Layout.leftMargin: 16
            }
            Label {
                text: darkItem.text
                font.pixelSize: 13
                color: darkItem.enabled ? Theme.consoleText : Theme.consoleMuted
                verticalAlignment: Text.AlignVCenter
                Layout.rightMargin: 16
                Layout.fillWidth: true
            }
        }
        background: Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            radius: 5
            color: darkItem.highlighted ? Theme.accent : "transparent"
        }
    }

    component DarkMenuSeparator: MenuSeparator {
        implicitHeight: 3
        contentItem: Rectangle {
            implicitHeight: 3
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.consoleBorder
        }
    }

    Menu {
        id: contextMenu
        topPadding: 6
        bottomPadding: 6
        background: Rectangle {
            implicitWidth: 200
            color: Theme.consoleBackground
            border.color: Theme.consoleBorder
            border.width: 1
            radius: 8
        }

        DarkMenuItem {
            text: qsTr("Copy")
            icon.source: "qrc:/icons/copy.svg"
            onTriggered: root.copyLog()
        }
        DarkMenuSeparator {}
        DarkMenuItem {
            text: qsTr("Clear log")
            icon.source: "qrc:/icons/clear.svg"
            onTriggered: AppController.logModel.clear()
        }
        DarkMenuItem {
            text: qsTr("Save log…")
            icon.source: "qrc:/icons/save.svg"
            onTriggered: root.saveLogRequested()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            color: Theme.consoleBackground
            border.width: 0
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.consoleBorder }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                Label {
                    text: qsTr("Data monitor")
                    font.pixelSize: 11
                    font.letterSpacing: 1
                    color: Theme.consoleMuted
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: qsTr("%1 lines").arg(root.formatCount(AppController.logModel.totalLineCount))
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.consoleMuted
                }
                Label {
                    text: qsTr("Rx %1").arg(root.formatBytes(AppController.logModel.rxBytes))
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.consoleMuted
                }
                Label {
                    text: qsTr("Tx %1").arg(root.formatBytes(AppController.logModel.txBytes))
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.consoleMuted
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.consoleBackground

            Flickable {
                id: flick
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                anchors.topMargin: 12
                anchors.bottomMargin: 12
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                contentWidth: root.wrapLines ? width : Math.max(width, contentEdit.paintedWidth)
                contentHeight: contentEdit.paintedHeight

                // Auto-scroll to the newest line unless the user paused or
                // has manually scrolled away from the bottom.
                property bool stickToBottom: true
                onContentHeightChanged: if (!root.paused && stickToBottom) contentY = Math.max(0, contentHeight - height)
                onContentYChanged: stickToBottom = (contentY + height >= contentHeight - 4)
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    contentItem: Rectangle {
                        implicitWidth: 8
                        radius: width / 2
                        color: Theme.consoleMuted
                        opacity: parent.pressed ? 0.9 : (parent.hovered ? 0.7 : 0.45)
                    }
                    background: Rectangle {
                        implicitWidth: 8
                        color: Theme.consoleBorder
                        opacity: 0.3
                    }
                }
                ScrollBar.horizontal: ScrollBar {
                    policy: ScrollBar.AsNeeded
                    visible: !root.wrapLines
                    contentItem: Rectangle {
                        implicitHeight: 8
                        radius: height / 2
                        color: Theme.consoleMuted
                        opacity: parent.pressed ? 0.9 : (parent.hovered ? 0.7 : 0.45)
                    }
                    background: Rectangle {
                        implicitHeight: 8
                        color: Theme.consoleBorder
                        opacity: 0.3
                    }
                }

                TextEdit {
                    id: contentEdit
                    width: root.wrapLines ? flick.width : implicitWidth
                    textFormat: TextEdit.PlainText
                    font.family: AppController.logFontFamily
                    font.pixelSize: AppController.logFontSize
                    color: Theme.consoleText
                    wrapMode: root.wrapLines ? TextEdit.Wrap : TextEdit.NoWrap

                    readOnly: true
                    selectByMouse: true
                    selectionColor: Theme.accent
                    selectedTextColor: Theme.accentForeground

                    function rebuild() { text = AppController.logModel.fullPlainDump() }
                    Component.onCompleted: rebuild()

                    Connections {
                        target: AppController.logModel
                        function onLineAppended(text) { contentEdit.insert(contentEdit.length, text) }
                        function onLineEvicted(docLength) { contentEdit.remove(0, docLength) }
                        function onRebuildNeeded() { contentEdit.rebuild() }
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: contextMenu.popup()
            }
        }
    }
}
