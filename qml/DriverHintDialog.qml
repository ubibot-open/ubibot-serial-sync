import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Opened by Main.qml in response to AppController.driverHintRequested() --
// see AppController::openPort() for exactly when that fires (no port
// selected, none available, and the CH340 driver doesn't look installed).
// See core/driver_check.h and docs/ch340-driver-hint.md for the detection
// itself and the reasoning behind it.
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
        // Hidden rather than disabled when no URL is configured yet (see
        // AppController.ch340DriverUrl/.env's own CH340_DRIVER_DOWNLOAD_URL)
        // -- a visibly-dead button invites clicking it to see what happens;
        // no button at all doesn't.
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
            text: qsTr("No serial port was found. If you've connected a UbiBot device and it still isn't showing up, you may need to install the CH340 USB-to-serial driver.")
        }

        CheckBox {
            text: qsTr("Don't show this again")
            onToggled: AppController.setSuppressCh340DriverHint(checked)
        }
    }
}
