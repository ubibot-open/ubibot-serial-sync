import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// "Check for software update" -- opened from Main.qml's Help menu, which
// also kicks off AppController.checkForAppUpdate() right before open()
// (this dialog itself never triggers a check -- it only ever reflects
// whatever AppController.appUpdateState/appUpdateMessage/etc. already are,
// so re-opening it after a check without re-triggering one just shows the
// last result again). See core/software_update_client.h and
// docs/app-self-update.md for the full mechanism this drives.
Dialog {
    id: root
    title: qsTr("Software Update")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 380
    palette: Theme.palette
    font.family: Theme.baseFontFamily
    font.pixelSize: Theme.baseFontSize
    background: DialogCard {}
    header: Label {
        text: root.title
        font.bold: true
        padding: 16
        color: Theme.text
    }

    // Force-updates (server-side is_force_update) can't be dismissed short
    // of actually updating -- no titlebar close glyph, no closing on Esc/
    // outside click, and the "Later" button below is hidden for the same
    // reason. Every other state closes normally.
    readonly property bool forced: AppController.appUpdateForced && AppController.appUpdateState === "updateAvailable"
    closePolicy: forced ? Popup.NoAutoClose : (Popup.CloseOnEscape | Popup.CloseOnPressOutside)

    footer: DialogButtonBox {
        palette: Theme.palette
        Button {
            text: qsTr("Later")
            palette: Theme.palette
            visible: AppController.appUpdateState === "updateAvailable" && !root.forced
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
        Button {
            text: qsTr("Cancel")
            palette: Theme.palette
            visible: AppController.appUpdateState === "downloading"
            onClicked: AppController.cancelAppUpdateDownload()
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
        Button {
            text: qsTr("Update now")
            palette: Theme.palette
            visible: AppController.appUpdateState === "updateAvailable"
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
            onClicked: AppController.installAppUpdate()
        }
        Button {
            text: qsTr("Retry")
            palette: Theme.palette
            visible: AppController.appUpdateState === "error"
            onClicked: AppController.checkForAppUpdate()
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
        }
        Button {
            text: qsTr("OK")
            palette: Theme.palette
            visible: AppController.appUpdateState === "upToDate" || AppController.appUpdateState === "error"
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
    }

    ColumnLayout {
        width: parent.width
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            visible: AppController.appUpdateState === "checking" || AppController.appUpdateState === "installing"
            spacing: 10
            BusyIndicator { implicitWidth: 20; implicitHeight: 20; running: parent.visible }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: AppController.appUpdateMessage
                color: Theme.text
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            visible: AppController.appUpdateState === "updateAvailable"
            spacing: 4
            Label {
                text: qsTr("Version %1 is available (you have %2)").arg(AppController.remoteAppVersion).arg(AppController.appVersion)
                font.bold: true
                color: Theme.text
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            Label {
                visible: root.forced
                text: qsTr("This update is required to keep using the app.")
                color: Theme.error
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                clip: true
                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: AppController.appUpdateMessage
                    color: Theme.textMuted
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            visible: AppController.appUpdateState === "downloading"
            spacing: 6
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: AppController.appUpdateMessage
                color: Theme.text
            }
            ProgressBar {
                Layout.fillWidth: true
                from: 0
                to: 1
                value: AppController.appUpdateProgress
            }
            Label {
                text: Math.round(AppController.appUpdateProgress * 100) + "%"
                color: Theme.textMuted
                font.pixelSize: Theme.baseFontSize - 1
            }
        }

        Label {
            Layout.fillWidth: true
            visible: AppController.appUpdateState === "upToDate" || AppController.appUpdateState === "error"
            wrapMode: Text.WordWrap
            text: AppController.appUpdateMessage
            color: AppController.appUpdateState === "error" ? Theme.error : Theme.text
        }
    }
}
