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

    // Main.qml listens for this and opens its own shared DriverListDialog
    // instance -- kept as one dialog app-wide (also reachable from the
    // Help menu) rather than a second copy nested in here, same pattern
    // CommandLibraryPanel.qml's own openParams(row) uses to reach a panel
    // that lives in Main.qml instead.
    signal downloadRequested()

    title: qsTr("Serial driver may be missing")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 380
    palette: Theme.palette
    font.family: Theme.baseFontFamily
    font.pixelSize: Theme.baseFontSize
    background: DialogCard {}
    // See ModalDim.qml -- keeps this modal's dimming out of the frameless
    // main window's shadow-margin ring.
    Overlay.modal: ModalDim {}
    header: Label {
        text: root.title
        font.bold: true
        padding: 16
        color: Theme.text
    }

    footer: DialogButtonBox {
        palette: Theme.palette
        // Opens DriverListDialog (see downloadRequested above) rather than
        // jumping straight to AppController.ch340DriverUrl itself -- always
        // visible now, since that dialog handles "no link for this
        // platform" per-row on its own instead of this button needing to
        // know about it.
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

        // Same setting Main.qml's banner "✕" writes -- whichever one the
        // user reaches for first, the other stays in sync (this one's
        // `checked:` binding, not just its onToggled, so reopening this
        // dialog after dismissing from the banner shows it already
        // ticked, rather than looking freshly unset).
        CheckBox {
            text: qsTr("Don't show this again")
            checked: AppController.ch340DriverHintDismissed
            onToggled: AppController.ch340DriverHintDismissed = checked
        }
    }
}
