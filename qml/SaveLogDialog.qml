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
    // Dialogs don't reliably inherit ApplicationWindow's own palette --
    // see Theme.qml. `palette` alone re-themes the controls inside but
    // Fusion's default Dialog background apparently doesn't rebind to
    // `palette.window` live, so the canvas itself needs an explicit
    // override too.
    palette: Theme.palette
    font.family: Theme.baseFontFamily
    font.pixelSize: Theme.baseFontSize
    // See DialogCard.qml for why this dialog needs its own elevated
    // surface + border + shadow instead of a plain Theme.background fill.
    background: DialogCard {}
    // See ModalDim.qml -- keeps this modal's dimming out of the frameless
    // main window's shadow-margin ring.
    Overlay.modal: ModalDim {}
    // Same rebind gap as `background` above, but for the title strip --
    // Fusion's default Dialog header is its own separately-drawn piece.
    // Uses Theme.surface (not Theme.background) to match DialogCard's fill
    // above so the header reads as part of the same panel, not a seam.
    header: Label {
        text: root.title
        font.bold: true
        padding: 14
        color: Theme.text
        background: Rectangle { color: Theme.surface }
    }

    property string errorText: ""

    onOpened: {
        fileNameField.text = AppController.suggestedLogBaseName() + ".log"
        locationField.text = AppController.suggestedLogDirectory()
        formatGroup.checkedButton = plainRadio
        autoRotateCheck.checked = false
        errorText = ""
    }

    contentItem: ColumnLayout {
        spacing: 14

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 10
            rowSpacing: 8

            Label { text: qsTr("File name") }
            TextField { id: fileNameField; Layout.fillWidth: true; font.family: Theme.monoFont; palette: Theme.palette }

            Label { text: qsTr("Location") }
            RowLayout {
                Layout.fillWidth: true
                TextField { id: locationField; Layout.fillWidth: true; font.family: Theme.monoFont; palette: Theme.palette }
                Button { text: qsTr("Browse…"); palette: Theme.palette; onClicked: folderDialog.open() }
            }
        }

        // Explicit `palette:` on the GroupBox and each control below for the
        // same reason the footer buttons need it -- see the footer comment.
        GroupBox {
            title: qsTr("Format")
            Layout.fillWidth: true
            palette: Theme.palette
            RowLayout {
                anchors.fill: parent
                ButtonGroup { id: formatGroup }
                RadioButton { id: plainRadio; text: qsTr("Plain text (.log)"); palette: Theme.palette; ButtonGroup.group: formatGroup; property string value: "text" }
                RadioButton { id: csvRadio; text: "CSV"; palette: Theme.palette; ButtonGroup.group: formatGroup; property string value: "csv" }
                RadioButton { id: hexRadio; text: qsTr("HEX dump"); palette: Theme.palette; ButtonGroup.group: formatGroup; property string value: "hex" }
            }
        }

        CheckBox {
            id: autoRotateCheck
            text: qsTr("Continue logging to disk, rotating the file every day")
            palette: Theme.palette
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
        // Same rebind gap as `background`/`header` above -- an explicit
        // `footer:` gets its own separately-drawn Fusion background too.
        // Theme.surface again, matching DialogCard/header.
        background: Rectangle { color: Theme.surface }
        // A Popup's children apparently don't reliably pick up a *live*
        // change to an inherited palette either (only their own direct
        // assignment reacts) -- see Theme.qml -- so this box and each of
        // its buttons need their own explicit `palette:` too.
        palette: Theme.palette
        Button { text: qsTr("Cancel"); palette: Theme.palette; DialogButtonBox.buttonRole: DialogButtonBox.RejectRole }
        Button {
            text: qsTr("Save")
            highlighted: true
            palette: Theme.palette
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
