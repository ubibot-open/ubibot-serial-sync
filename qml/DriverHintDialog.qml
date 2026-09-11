// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Opened by clicking Main.qml's own driver-hint banner (shown whenever
// AppController.ch340DriverInstalled is false and the banner hasn't been
// dismissed) -- not popped up on its own. See core/driver_check.h and
// docs/ch340-driver-hint.md for the detection itself and the reasoning
// behind checking once at launch and surfacing it as a persistent banner
// rather than interrupting the user with this dialog the moment they
// happen to hit the wall it explains.
Dialog {
    id: root

    signal downloadRequested()

    title: qsTr("Serial driver may be missing")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 380
    palette: Theme.palette
    font.family: Theme.baseFontFamily
    font.pixelSize: Theme.baseFontSize
    background: DialogCard {}
    Overlay.modal: ModalDim {}
    header: Label {
        text: root.title
        font.bold: true
        padding: 16
        color: Theme.text
    }

    footer: DialogButtonBox {
        palette: Theme.palette
        Button {
            text: qsTr("Download driver")
            palette: Theme.palette
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
            onClicked: root.downloadRequested()
        }
        Button {
            text: qsTr("Close")
            palette: Theme.palette
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
    }

    ColumnLayout {
        width: parent.width
        spacing: 12

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.text
            text: qsTr("The CH340 USB-to-serial driver doesn't appear to be installed on this computer. Without it, a connected UbiBot device won't show up as an available serial port.")
        }
        CheckBox {
            text: qsTr("Don't show this again")
            checked: AppController.ch340DriverHintDismissed
            onToggled: AppController.ch340DriverHintDismissed = checked
        }
    }
}
