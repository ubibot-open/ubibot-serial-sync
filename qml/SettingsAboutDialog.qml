import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// "Settings & About" dialog: interface language switch plus static
// version/library/support info. The language list comes from
// AppController.availableLanguages() -- a dropdown rather than radio
// buttons, since it's meant to grow past a dozen entries.
Dialog {
    id: root
    title: qsTr("Settings & About")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 460
    standardButtons: Dialog.Close

    contentItem: ColumnLayout {
        spacing: 14

        GroupBox {
            title: qsTr("Interface language")
            Layout.fillWidth: true

            ComboBox {
                id: languageCombo
                anchors.fill: parent
                textRole: "nativeName"
                valueRole: "code"
                model: AppController.availableLanguages()
                Component.onCompleted: currentIndex = indexOfValue(AppController.currentLanguage)
                onActivated: AppController.currentLanguage = currentValue
            }
        }

        GroupBox {
            title: qsTr("Command library")
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                Label { text: AppController.libraryVersion; font.family: Theme.monoFont }
                Label {
                    text: qsTr("Up to date")
                    color: Theme.accent800
                    padding: 4
                    background: Rectangle { color: Theme.accentTint }
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Check for updates")
                    onClicked: updateResultDialog.open()
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 16
            rowSpacing: 4

            Label { text: qsTr("Version"); font.pixelSize: 11; color: Theme.textMuted }
            Label { text: qsTr("Platform"); font.pixelSize: 11; color: Theme.textMuted }
            Label { text: "1.0.0 (Qt 6.11)" }
            Label { text: "Windows · macOS · Linux" }

            Label { text: qsTr("Support"); font.pixelSize: 11; color: Theme.textMuted }
            Label { text: qsTr("Devices"); font.pixelSize: 11; color: Theme.textMuted }
            Label { text: "support@ubibot.com" }
            Label { text: qsTr("%1 models · %2 commands").arg(AppController.modelCount).arg(AppController.commandCount) }
        }
    }

    Dialog {
        id: updateResultDialog
        title: qsTr("Command library")
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok
        Label {
            width: 320
            wrapMode: Text.WordWrap
            text: AppController.checkForLibraryUpdate()
        }
    }
}
