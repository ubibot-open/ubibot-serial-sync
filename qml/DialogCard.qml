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
//
// Fix: fill with `Theme.surface` (a real tonal step up from the window
// background, already used elsewhere for "raised panel" chrome), a
// stronger `Theme.dialogBorder`, and a soft drop shadow.
//
// The shadow used to be a handful of stacked, oversized, low-opacity plain
// Rectangles rather than a real blur -- cheaper, and with no GPU
// shader-effect-source lifetime subtleties to get wrong, but per user
// feedback it showed up as a few visible hard-edged "steps" in light mode
// instead of one smooth falloff (unavoidable with that technique -- a
// handful of flat-edged rectangles can only ever approximate a blur, never
// actually be one, no matter how their opacities/margins are tuned).
// MultiEffect's shadowEnabled is a real GPU-blurred shadow -- the same kind
// of soft, continuous falloff a native Windows/macOS window shadow has --
// and has shipped as a stable part of Qt Quick's own "essential" install
// (no separate aqt module, see .github/workflows/release.yml's own comment
// about which modules are and aren't bundled) since Qt 6.5, well under this
// project's Qt 6.7 floor.
//
// `card` itself is `visible: false` -- it's only ever drawn *through* the
// MultiEffect below, which already renders an unmodified copy of its
// `source` plus the shadow behind it, so drawing `card` a second time on
// top would be redundant. An item stays a valid effect source while
// invisible: Qt Quick's layering (which MultiEffect uses internally)
// captures an item's rendering into its texture regardless of the item's
// own `visible` value -- only *direct* on-screen drawing respects
// `visible`.
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
        // Without this, the blurred shadow would be cut off right at
        // card's own bounds instead of being free to spread past them.
        autoPaddingEnabled: true
        shadowEnabled: true
        shadowColor: "black"
        // Kept subtle on purpose -- this is a small settings dialog, not a
        // modal warning; a heavier shadow would draw more attention than
        // the dialog's own content deserves. Dark mode's already-near-black
        // window means this reads as faint there too, same as a real OS
        // shadow barely showing up against a dark desktop.
        shadowOpacity: 0.35
        shadowBlur: 0.6
        shadowHorizontalOffset: 0
        shadowVerticalOffset: 3
    }
}
