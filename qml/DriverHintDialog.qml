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
        // Hidden rather than disabled when no URL is available for this
        // platform (see AppController.ch340DriverUrl /
        // DriverCheck::downloadUrl()) -- a visibly-dead button invites
        // clicking it to see what happens; no button at all doesn't.
        Button {
            text: qsTr("Download driver")
            palette: Theme.palette
            visible: AppController.ch340DriverUrl.length > 0
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
            onClicked: Qt.openUrlExternally(AppController.ch340DriverUrl)
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
