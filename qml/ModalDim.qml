// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Window
import UbiBot

Rectangle {
    color: "transparent"

    Rectangle {
        anchors.fill: parent
        anchors.margins: Window.window ? Window.window.activeMargin : 0
        color: "#80000000"
    }
}
