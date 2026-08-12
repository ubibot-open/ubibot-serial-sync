import QtQuick
import UbiBot

// Shared "elevated card" background for every Dialog/Popup's `background:`
// property (Main.qml's historyPopup/portErrorDialog, ConnectionWizardDialog,
// SettingsAboutDialog + its nested updateResultDialog, SaveLogDialog,
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
//
// Fix: fill with `Theme.surface` (a real tonal step up from the window
// background, already used elsewhere for "raised panel" chrome), a
// stronger `Theme.dialogBorder`, and a soft drop shadow. The shadow is a
// handful of stacked, oversized, low-opacity Rectangles rather than a
// QtQuick.Effects.MultiEffect blur -- cheaper, has no GPU-shader-source
// lifetime subtleties to get wrong, and is plenty for "this is a raised
// panel" at dialog scale.
//
// Popup does not clip its `background` item by default, so these
// intentionally-oversized rectangles are free to bleed a few pixels past
// the dialog's own bounds instead of being cut off at the edge.
Item {
    id: root

    Rectangle {
        anchors.fill: parent
        anchors.margins: -7
        radius: 3
        color: "black"
        opacity: 0.06
    }
    Rectangle {
        anchors.fill: parent
        anchors.margins: -4
        radius: 2
        color: "black"
        opacity: 0.10
    }
    Rectangle {
        anchors.fill: parent
        anchors.margins: -2
        color: "black"
        opacity: 0.16
    }

    Rectangle {
        id: card
        anchors.fill: parent
        color: Theme.surface
        border.color: Theme.dialogBorder
        border.width: 1
    }
}
