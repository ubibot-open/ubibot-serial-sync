// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Effects
import UbiBot

// Shared "elevated card" background for every Dialog/Popup's `background:`
// property (Main.qml's historyPopup/portErrorDialog, SettingsAboutDialog +
// its nested updateResultDialog, SaveLogDialog, AboutDialog,
// RemoteAssistPanel's notImplementedDialog). Replaces the old
// `Rectangle { color: Theme.background; border.color: Theme.divider; ... }`
// pattern repeated in each of those files, which had two problems:
//
// 1. The dialog's fill was literally the same color as the main window
//    behind it (both `Theme.background`), so nothing but a 1px border
//    marked where one ended and the other began.
// 2. That border used `Theme.divider`, a deliberately quiet color meant for
//    in-page separators, not for telling a whole floating panel apart from
//    the app behind it -- in dark mode it's only a few shades off
//    `Theme.background`, so the two blended together completely.

Item {
    id: root

    Rectangle {
        id: card
        anchors.fill: parent
        visible: false
        color: Theme.surface
        border.color: Theme.dialogBorder
        border.width: 1
    }

    MultiEffect {
        anchors.fill: card
        source: card
        autoPaddingEnabled: true
        shadowEnabled: true
        shadowColor: "black"
        shadowOpacity: 0.35
        shadowBlur: 0.6
        shadowHorizontalOffset: 0
        shadowVerticalOffset: 3
    }
}
