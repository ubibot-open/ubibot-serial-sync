import QtQuick
import QtQuick.Window
import UbiBot

// Shared `Overlay.modal:` background for every modal Dialog/Popup in the
// app (see each one's own `Overlay.modal: ModalDim {}`) -- Qt's own default
// modal dimmer always fills the whole window, which on this app's
// frameless main window includes the transparent shadowMargin ring around
// Main.qml's `frame` (see that property's own comment there). Left as Qt's
// default, opening any modal dialog visibly dimmed that ring too, which
// read as "the shadow got dirty" rather than "a dialog opened". This insets
// the actual dimming to whatever `frame` currently occupies instead: a
// transparent outer layer (Qt still sizes this to fill the whole window,
// same as its own default dimmer would) with just an inset inner Rectangle
// carrying real color.
//
// `Overlay.modal`/`.modeless` are attached properties read from the
// individual Popup/Dialog being shown, not something settable once on the
// window and inherited by every popup in it (confirmed against Qt's own
// docs after an earlier attempt at exactly that had no visible effect) --
// every modal Dialog/Popup in the app sets this individually instead.
//
// `Window.window` (not a plain `window` id -- most of the files that need
// this live in their own separate .qml document, where Main.qml's own
// `window` id isn't in scope) resolves to the actual ApplicationWindow
// instance once this item is actually parented into that window's overlay;
// activeMargin is a plain QML property on it (see Main.qml), reachable by
// name through ordinary dynamic property lookup even without a
// C++-registered type behind it. Falls back to no inset (0) in the
// vanishingly unlikely case this ever renders before that resolves, rather
// than erroring out.
Rectangle {
    color: "transparent"

    Rectangle {
        anchors.fill: parent
        anchors.margins: Window.window ? Window.window.activeMargin : 0
        // Reproduces Qt Quick Controls' own default modal-dim appearance
        // (black at 50% opacity) -- this file exists to relocate that
        // dimming, not to restyle it.
        color: "#80000000"
    }
}
