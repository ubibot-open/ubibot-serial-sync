import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// The parameter-entry strip shown above the send box when the user picks a
// command from CommandLibraryPanel that needs arguments (e.g.
// "AT+INTERVAL=<sec>"). openForRow() is called by Main.qml in response to
// CommandLibraryPanel.openParams.
Rectangle {
    id: root
    visible: false
    color: Theme.background
    border.color: Theme.divider
    border.width: 1
    implicitHeight: visible ? column.implicitHeight + 24 : 0

    property int row: -1
    property string commandName: ""
    property var params: []
    property var values: ({})

    function openForRow(rowIndex) {
        row = rowIndex
        params = AppController.paramsForRow(rowIndex)
        var initial = {}
        for (var i = 0; i < params.length; ++i) initial[params[i].key] = params[i].defaultValue
        values = initial
        commandName = AppController.commandNameForRow(rowIndex)
        visible = true
    }

    function updateValue(key, value) {
        var v = values
        v[key] = value
        values = v
    }

    readonly property string preview: row >= 0 ? AppController.previewCommand(row, values) : ""

    ColumnLayout {
        id: column
        anchors.fill: parent
        anchors.margins: 14
        spacing: 13

        RowLayout {
            Layout.fillWidth: true
            Label { text: root.commandName; font.bold: true; font.pixelSize: Theme.baseFontSize + 3 }
            Label {
                text: qsTr("Needs parameters")
                color: Theme.accent
                font.pixelSize: Theme.baseFontSize - 1
                padding: 4
                background: Rectangle { border.color: Theme.accent; border.width: 1; color: "transparent" }
            }
            Item { Layout.fillWidth: true }
            Button { text: qsTr("Cancel"); onClicked: root.visible = false }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 10

            Repeater {
                model: root.params
                delegate: ColumnLayout {
                    required property var modelData
                    spacing: 2
                    Layout.fillWidth: true

                    Label { text: modelData.label; font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
                    TextField {
                        Layout.fillWidth: true
                        placeholderText: modelData.hint
                        // Fusion's default placeholder color barely shows up
                        // against this field's dark-mode background -- see
                        // Main.qml's inputField for the same fix.
                        placeholderTextColor: Theme.textMuted
                        text: root.values[modelData.key] !== undefined ? root.values[modelData.key] : ""
                        font.family: Theme.monoFont
                        onTextEdited: root.updateValue(modelData.key, text)
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: root.preview
                font.family: Theme.monoFont
                font.pixelSize: Theme.baseFontSize
                color: Theme.accent700
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Button {
                // Was "Send" -- this panel (like the rest of the device
                // command library) only stages text into the manual-send
                // box now, it doesn't write to the port itself. See
                // AppController::loadCommandWithParamsIntoDraft.
                text: qsTr("Insert")
                highlighted: true
                onClicked: {
                    AppController.loadCommandWithParamsIntoDraft(root.row, root.values)
                    root.visible = false
                }
            }
        }
    }
}
