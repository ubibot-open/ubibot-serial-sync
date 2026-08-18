import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Left-hand panel for "Remote support" mode.
//
// NOTE: this is a UI placeholder, same as the old Widgets version. The
// design calls for a support agent to enter a code and connect directly to
// this app, which needs a relay/signaling server or a peer-to-peer
// transport -- neither exists yet. Everything here (code/OTP generation,
// permission toggles) is cosmetic local state; "Start session" says so
// explicitly instead of pretending to connect anyone. There's no backend
// logic to speak of, so unlike the other panels this one has no
// AppController involvement at all.
Flickable {
    id: root
    contentWidth: width
    contentHeight: column.implicitHeight + 32
    clip: true

    function randomGroup() {
        return String(1000 + Math.floor(Math.random() * 9000))
    }
    function regenerate() {
        code = randomGroup() + "-" + randomGroup() + "-" + randomGroup()
        otp = randomGroup()
    }

    property string code: ""
    property string otp: ""
    Component.onCompleted: regenerate()

    ColumnLayout {
        id: column
        width: root.width
        anchors.margins: 18
        spacing: 16

        Label { text: qsTr("Remote support"); font.bold: true }
        Label {
            text: qsTr("Send the code below to UbiBot support so they can connect to your computer and help diagnose device issues.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: Theme.baseFontSize
            color: Theme.textMuted
        }

        Rectangle {
            Layout.fillWidth: true
            border.color: Theme.divider
            border.width: 1
            color: "transparent"
            implicitHeight: codeColumn.implicitHeight + 24

            ColumnLayout {
                id: codeColumn
                anchors.centerIn: parent
                width: parent.width - 32
                spacing: 10

                Label {
                    text: qsTr("Your code")
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: Theme.baseFontSize - 2
                    font.letterSpacing: 1
                    color: Theme.textMuted
                }
                Label {
                    text: root.code
                    Layout.alignment: Qt.AlignHCenter
                    font.family: Theme.monoFont
                    font.pixelSize: Theme.baseFontSize + 14
                    font.letterSpacing: 3
                    color: Theme.accent800
                }
                Label {
                    text: qsTr("Valid for 10 minutes · this session only")
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: Theme.baseFontSize - 1
                    color: Theme.textMuted
                }
                RowLayout {
                    Layout.fillWidth: true
                    Button {
                        text: qsTr("Copy code")
                        Layout.fillWidth: true
                        onClicked: { clipboardHelper.text = root.code; clipboardHelper.selectAll(); clipboardHelper.copy() }
                    }
                    Button { text: qsTr("Regenerate"); Layout.fillWidth: true; onClicked: root.regenerate() }
                }
            }
        }

        // Off-screen helper: QML has no clipboard API of its own, but a
        // hidden TextEdit's copy() does the job without any C++.
        TextEdit { id: clipboardHelper; visible: false }

        Label { text: qsTr("One-time password"); font.pixelSize: Theme.baseFontSize; color: Theme.textMuted }
        TextField {
            Layout.fillWidth: true
            text: root.otp
            readOnly: true
            horizontalAlignment: TextInput.AlignHCenter
            font.family: Theme.monoFont
            font.letterSpacing: 5
        }

        CheckBox { text: qsTr("Allow support to send/receive on the serial port"); checked: true }
        CheckBox { text: qsTr("Share this session's log"); checked: true }
        CheckBox { text: qsTr("Allow full desktop control"); checked: false }

        Button {
            text: qsTr("Start session")
            highlighted: true
            Layout.fillWidth: true
            onClicked: notImplementedDialog.open()
        }

        Label { text: qsTr("Not connected"); font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
    }

    Dialog {
        id: notImplementedDialog
        title: qsTr("Remote support")
        modal: true
        anchors.centerIn: Overlay.overlay
        palette: Theme.palette
        font.family: Theme.baseFontFamily
        font.pixelSize: Theme.baseFontSize
        // See DialogCard.qml for why this dialog needs its own elevated
        // surface + border + shadow instead of a plain Theme.background fill.
        background: DialogCard {}
        header: Label {
            text: notImplementedDialog.title
            font.bold: true
            padding: 14
            color: Theme.text
            background: Rectangle { color: Theme.surface }
        }
        // Was `standardButtons: Dialog.Ok` -- see SettingsAboutDialog.qml's
        // root footer for why that auto-generated button had to be spelled
        // out explicitly instead (needs its own `palette:`).
        footer: DialogButtonBox {
            palette: Theme.palette
            background: Rectangle { color: Theme.surface }
            Button {
                text: qsTr("OK")
                palette: Theme.palette
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
        }
        Label {
            width: 320
            wrapMode: Text.WordWrap
            text: qsTr("Remote support requires a signaling/relay service that this build does not include yet. The session code and permissions above are ready to wire up once that transport exists.")
            color: Theme.text
        }
    }
}
