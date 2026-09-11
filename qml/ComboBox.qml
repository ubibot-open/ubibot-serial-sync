// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Controls.Fusion as Fusion
import UbiBot

Fusion.ComboBox {
    id: control
    leftPadding: 8
    implicitHeight: Math.max(32, implicitBackgroundHeight + topInset + bottomInset,
                              implicitContentHeight + topPadding + bottomPadding,
                              implicitIndicatorHeight + topPadding + bottomPadding)

    Binding { target: control.popup.background; property: "border.color"; value: Theme.dialogBorder }
    Connections {
        target: control.popup
        function onOpened() {
            control.popup.contentItem.positionViewAtIndex(control.currentIndex, ListView.Center)
        }
    }

    MouseArea {
        z: 10
        anchors.fill: parent
        enabled: control.count === 0
        cursorShape: Qt.ArrowCursor
    }

    delegate: Fusion.MenuItem {
        id: menuItem
        required property var model
        required property int index
        readonly property bool isCurrent: control.currentIndex === index

        width: ListView.view.width
        text: model[control.textRole]
        font.weight: isCurrent ? Font.DemiBold : Font.Normal
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: 20
            color: menuItem.down || menuItem.highlighted
                   ? Fusion.Fusion.highlight(control.palette)
                   : (menuItem.isCurrent ? Theme.accentTint : "transparent")

            Rectangle {
                visible: menuItem.isCurrent
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 3
                color: Theme.accent800
            }
        }
    }
}
