// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

ComboBox {
    id: control

    model: AppController.portListModel
    displayText: currentIndex >= 0 ? AppController.portListModel.portNameAt(currentIndex)
                                    : qsTr("No ports found")
    popup.width: Math.max(control.width, 280)
    popup.background: DialogCard {}
    delegate: ItemDelegate {
        id: portDelegate
        required property int index
        required property string portName
        required property string description
        required property bool recommended
        required property string chipLabel
        width: control.popup.width
        highlighted: control.highlightedIndex === index
        // ItemDelegate's own implicit height ignores whatever's stuffed
        // into it below -- size off the row's own implicit height instead.
        implicitHeight: portRow.implicitHeight + 16
        readonly property color rowText: highlighted ? Theme.accentForeground : Theme.text
        readonly property color rowMuted: highlighted ? Theme.accentForeground : Theme.textMuted
        readonly property color rowAccent: highlighted ? Theme.accentForeground : Theme.accent

        RowLayout {
            id: portRow
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 10
            Rectangle {
                Layout.preferredWidth: 3
                Layout.fillHeight: true
                Layout.topMargin: 2
                Layout.bottomMargin: 2
                radius: 1.5
                color: portDelegate.recommended ? portDelegate.rowAccent : "transparent"
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1
                Label {
                    text: portDelegate.portName
                    font.bold: true
                    color: portDelegate.rowText
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Label {
                    text: portDelegate.description.length > 0 ? portDelegate.description : qsTr("Unknown device")
                    font.pixelSize: Theme.baseFontSize - 1
                    color: portDelegate.rowMuted
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
            Label {
                visible: portDelegate.chipLabel.length > 0
                text: portDelegate.chipLabel
                font.bold: true
                color: portDelegate.rowAccent
                font.pixelSize: Theme.baseFontSize - 1
            }
        }
    }
}
