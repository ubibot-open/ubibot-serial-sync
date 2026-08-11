import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import UbiBot

// "Save session log" dialog: exports everything currently in the data
// monitor to a file immediately, and optionally turns on continuous,
// daily-rotating logging for the rest of the session.
Dialog {
    id: root
    title: qsTr("Save session log")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 460

    property string errorText: ""

    onOpened: {
        fileNameField.text = AppController.suggestedLogBaseName() + ".log"
        locationField.text = AppController.suggestedLogDirectory()
        formatGroup.checkedButton = plainRadio
        autoRotateCheck.checked = false
        errorText = ""
    }

    contentItem: ColumnLayout {
        spacing: 12

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 10
            rowSpacing: 8

            Label { text: qsTr("File name") }
            TextField { id: fileNameField; Layout.fillWidth: true; font.family: Theme.monoFont }

            Label { text: qsTr("Location") }
            RowLayout {
                Layout.fillWidth: true
                TextField { id: locationField; Layout.fillWidth: true; font.family: Theme.monoFont }
                Button { text: qsTr("Browse…"); onClicked: folderDialog.open() }
            }
        }

        GroupBox {
            title: qsTr("Format")
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                ButtonGroup { id: formatGroup }
                RadioButton { id: plainRadio; text: qsTr("Plain text (.log)"); ButtonGroup.group: formatGroup; property string value: "text" }
                RadioButton { id: csvRadio; text: "CSV"; ButtonGroup.group: formatGroup; property string value: "csv" }
                RadioButton { id: hexRadio; text: qsTr("HEX dump"); ButtonGroup.group: formatGroup; property string value: "hex" }
            }
        }

        CheckBox {
            id: autoRotateCheck
            text: qsTr("Continue logging to disk, rotating the file every day")
        }

        Label {
            visible: root.errorText.length > 0
            text: root.errorText
            color: Theme.error
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    footer: DialogButtonBox {
        Button { text: qsTr("Cancel"); DialogButtonBox.buttonRole: DialogButtonBox.RejectRole }
        Button {
            text: qsTr("Save")
            highlighted: true
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            onClicked: {
                var baseName = fileNameField.text
                var dot = baseName.lastIndexOf(".")
                if (dot > 0) baseName = baseName.substring(0, dot)

                var format = formatGroup.checkedButton ? formatGroup.checkedButton.value : "text"
                var error = AppController.saveLog(locationField.text, baseName, format, autoRotateCheck.checked)
                if (error.length > 0) root.errorText = error
                else root.accept()
            }
        }
    }

    FolderDialog {
        id: folderDialog
        currentFolder: "file:///" + locationField.text
        onAccepted: locationField.text = String(selectedFolder).replace("file:///", "")
    }
}
