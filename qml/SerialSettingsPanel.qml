import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Left-hand panel for "Serial" mode: port/baud/frame settings plus
// receive/transmit display options. Main.qml reads the selected* properties
// when the user hits "Open port".
Flickable {
    id: root
    contentWidth: width
    contentHeight: column.y + column.implicitHeight + 14
    clip: true

    readonly property string selectedPort: portCombo.currentIndex >= 0
        ? AppController.portListModel.portNameAt(portCombo.currentIndex) : ""
    readonly property int selectedBaud: parseInt(baudCombo.editText || baudCombo.currentText || "115200")
    readonly property int selectedDataBits: dataBitsCombo.currentValue
    readonly property int selectedParity: parityCombo.currentValue
    readonly property int selectedStopBits: stopBitsCombo.currentValue
    readonly property int selectedFlowControl: flowCombo.currentValue
    // Purely a display concern (no serial behavior implied), so it lives
    // entirely in QML rather than round-tripping through AppController.
    property bool wrapLines: true

    // Called both at startup and on every language change. The combos' data
    // (QSerialPort enum values) doesn't depend on language, only the
    // labels do -- so this preserves whatever the user already picked by
    // value, re-locating it in the freshly-relabeled model, instead of
    // resetting every field to its default each time the language changes.
    function rebuildOptions() {
        const previousDataBits = dataBitsCombo.currentValue
        const previousParity = parityCombo.currentValue
        const previousStopBits = stopBitsCombo.currentValue
        const previousFlowControl = flowCombo.currentValue

        dataBitsCombo.model = SerialOptions.dataBitsOptions()
        parityCombo.model = SerialOptions.parityOptions()
        stopBitsCombo.model = SerialOptions.stopBitsOptions()
        flowCombo.model = SerialOptions.flowControlOptions()

        dataBitsCombo.currentIndex = previousDataBits !== undefined ? dataBitsCombo.indexOfValue(previousDataBits) : 3
        parityCombo.currentIndex = previousParity !== undefined ? parityCombo.indexOfValue(previousParity) : 0
        stopBitsCombo.currentIndex = previousStopBits !== undefined ? stopBitsCombo.indexOfValue(previousStopBits) : 0
        flowCombo.currentIndex = previousFlowControl !== undefined ? flowCombo.indexOfValue(previousFlowControl) : 0
    }

    Connections {
        target: AppController
        function onCurrentLanguageChanged() { root.rebuildOptions() }
    }
    Component.onCompleted: rebuildOptions()

    // Design lays these three groups out as flat sections -- a small-caps
    // heading directly above its rows, no surrounding border/box -- rather
    // than GroupBox's bordered frame, so the heading style is hand-rolled
    // to match instead of relying on GroupBox.title.
    component SectionHeading: Label {
        Layout.fillWidth: true
        Layout.topMargin: 6
        font.pixelSize: 11
        font.letterSpacing: 1
        color: Theme.textMuted
    }

    ColumnLayout {
        id: column
        x: 14
        y: 14
        width: root.width - 28
        spacing: 18

        SectionHeading { text: qsTr("Port · PORT") }

        GridLayout {
            Layout.fillWidth: true
            enabled: !AppController.portOpen
            columns: 2
            columnSpacing: 10
            rowSpacing: 8

            Label { Layout.preferredWidth: 70; text: qsTr("Port") }
            RowLayout {
                Layout.fillWidth: true
                ComboBox {
                    id: portCombo
                    Layout.fillWidth: true
                    textRole: "displayLabel"
                    model: AppController.portListModel
                }
                ToolButton {
                    icon.source: "qrc:/icons/refresh.svg"
                    onClicked: AppController.portListModel.refresh()
                }
            }

            Label { Layout.preferredWidth: 70; text: qsTr("Baud rate") }
            ComboBox {
                id: baudCombo
                Layout.fillWidth: true
                editable: true
                model: SerialOptions.baudRateOptions()
                currentIndex: 3 // 115200
                validator: IntValidator { bottom: 50; top: 4000000 }
            }

            Label { Layout.preferredWidth: 70; text: qsTr("Data bits") }
            ComboBox {
                id: dataBitsCombo
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
                // Initial selection (and re-selection after a language
                // change) is handled by rebuildOptions().
            }

            Label { Layout.preferredWidth: 70; text: qsTr("Parity") }
            ComboBox {
                id: parityCombo
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
            }

            Label { Layout.preferredWidth: 70; text: qsTr("Stop bits") }
            ComboBox {
                id: stopBitsCombo
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
            }

            Label { Layout.preferredWidth: 70; text: qsTr("Flow control") }
            ComboBox {
                id: flowCombo
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
            }
        }

        SectionHeading { text: qsTr("Receive · RECEIVE") }

        RowLayout {
            RadioButton { text: "ASCII"; checked: !AppController.logModel.hexMode; onToggled: if (checked) AppController.logModel.hexMode = false }
            RadioButton { text: "HEX"; checked: AppController.logModel.hexMode; onToggled: if (checked) AppController.logModel.hexMode = true }
        }
        CheckBox {
            text: qsTr("Show timestamp")
            checked: AppController.logModel.showTimestamp
            onToggled: AppController.logModel.showTimestamp = checked
        }
        CheckBox {
            text: qsTr("Wrap lines")
            checked: root.wrapLines
            onToggled: root.wrapLines = checked
        }
        CheckBox {
            id: echoCheck
            text: qsTr("Echo sent data")
            checked: AppController.echoTx
            onToggled: AppController.echoTx = checked
        }

        SectionHeading { text: qsTr("Transmit · TRANSMIT") }

        RowLayout {
            RadioButton { text: "ASCII"; checked: !AppController.sendAsHex; onToggled: if (checked) AppController.sendAsHex = false }
            RadioButton { text: "HEX"; checked: AppController.sendAsHex; onToggled: if (checked) AppController.sendAsHex = true }
        }
        RowLayout {
            CheckBox {
                text: qsTr("Repeat send")
                checked: AppController.repeatSendEnabled
                onToggled: AppController.repeatSendEnabled = checked
            }
            SpinBox {
                from: 50
                to: 3600000
                stepSize: 50
                value: AppController.repeatIntervalMs
                enabled: AppController.repeatSendEnabled
                onValueModified: AppController.repeatIntervalMs = value
            }
            Label { text: qsTr("ms") }
        }
    }
}
