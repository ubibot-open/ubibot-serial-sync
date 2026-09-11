import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Opened from Main.qml's Help menu ("Driver downloads", always available
// regardless of whether the driver-hint banner has been dismissed -- see
// docs/ch340-driver-hint.md) and from DriverHintDialog.qml's own "Download
// driver" button. Lists every driver AppController.availableDrivers()
// knows about (just CH340 today) with its detected install state and a
// per-row Download button -- built as a list from the start, even with one
// entry, so a future driver (CP210x, FTDI, ...) is "one more entry in
// DriverCheck::knownDrivers()", not a second dialog or a reworked one.
Dialog {
    id: root
    title: qsTr("Driver downloads")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 420
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
        Button {
            text: qsTr("Close")
            palette: Theme.palette
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
    }

    // Re-read every time the dialog opens rather than bound live -- install
    // state itself is only ever checked once per process (see
    // AppController.ch340DriverInstalled), so there's nothing to react to
    // mid-session; onAboutToShow just avoids paying for the (cheap, but
    // non-zero) file-existence checks while the dialog isn't even open.
    property var drivers: []
    onAboutToShow: drivers = AppController.availableDrivers()

    ColumnLayout {
        width: parent.width
        spacing: 0

        Repeater {
            model: root.drivers

            delegate: ColumnLayout {
                id: driverRow
                required property var modelData
                required property int index
                Layout.fillWidth: true
                spacing: 0

                Rectangle { visible: driverRow.index > 0; Layout.fillWidth: true; height: 1; color: Theme.divider }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 10
                    Layout.bottomMargin: 10
                    spacing: 12

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label {
                            text: driverRow.modelData.name
                            font.bold: true
                            color: Theme.text
                        }
                        Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            color: Theme.textMuted
                            font.pixelSize: Theme.baseFontSize - 1
                            text: qsTr(driverRow.modelData.description)
                        }
                        Label {
                            color: driverRow.modelData.installed ? Theme.accent700 : Theme.error
                            font.pixelSize: Theme.baseFontSize - 1
                            text: driverRow.modelData.installed ? qsTr("Installed") : qsTr("Not detected")
                        }
                    }

                    Button {
                        text: qsTr("Download")
                        // Hidden (not disabled) when this platform has no
                        // known link for this driver -- same reasoning as
                        // DriverHintDialog.qml's own Download button.
                        visible: driverRow.modelData.url.length > 0
                        onClicked: Qt.openUrlExternally(driverRow.modelData.url)
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 8
            visible: root.drivers.length === 0
            wrapMode: Text.WordWrap
            color: Theme.textMuted
            text: qsTr("No drivers are relevant on this platform.")
        }
    }
}
