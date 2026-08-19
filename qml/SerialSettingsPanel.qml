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
    readonly property int selectedBaud: parseInt(baudCombo.currentText || "115200")
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

    // See docs/device-json-protocol-schema.md#5 -- selecting a model with a
    // "serial" block in devices.json unconditionally overwrites these
    // fields with its recommended values, regardless of whatever the user
    // had picked before. A model without one (every existing AT device)
    // leaves these fields untouched.
    function applyModelSerialDefaults() {
        const d = AppController.serialDefaultsForModel(AppController.currentModelId)
        if (!d.present) return
        // baudCombo is selection-only (see its own comment below) -- find()
        // returns -1 (a blank/unselected box) for a rate that isn't one of
        // SerialOptions.baudRateOptions()'s fixed choices. Every model
        // shipped in devices.json today recommends 115200, which is one of
        // them; a future model recommending something else would need that
        // rate added to the fixed list too, or it'd show up unselected here.
        baudCombo.currentIndex = baudCombo.find(String(d.baudRate))
        dataBitsCombo.currentIndex = dataBitsCombo.indexOfValue(d.dataBits)
        parityCombo.currentIndex = parityCombo.indexOfValue(d.parity)
        stopBitsCombo.currentIndex = stopBitsCombo.indexOfValue(d.stopBits)
        flowCombo.currentIndex = flowCombo.indexOfValue(d.flowControl)
    }

    Connections {
        target: AppController
        function onCurrentLanguageChanged() { root.rebuildOptions() }
        function onCurrentModelChanged() { root.applyModelSerialDefaults() }
    }
    Component.onCompleted: {
        rebuildOptions()
        // Covers the case where the app starts up already on a model that
        // has serial defaults (e.g. it was the last one selected).
        applyModelSerialDefaults()
    }

    // Design lays these three groups out as flat sections -- a small-caps
    // heading directly above its rows, no surrounding border/box -- rather
    // than GroupBox's bordered frame, so the heading style is hand-rolled
    // to match instead of relying on GroupBox.title.
    component SectionHeading: Label {
        Layout.fillWidth: true
        Layout.topMargin: 8
        font.pixelSize: Theme.baseFontSize - 1
        font.letterSpacing: 1
        color: Theme.textMuted
    }

    // The Port/Baud rate/.../Flow control label column below used to pin
    // every row to a fixed Layout.preferredWidth: 70 -- comfortably wide
    // enough for the English/Chinese originals, but some of this app's
    // other shipped languages translate these into noticeably longer
    // words/phrases ("Flow control" -> "Управление потоком" in Russian,
    // say), which just got clipped at that fixed width.
    //
    // Letting the column auto-size to its widest label (no preferredWidth
    // at all) fixed that clipping, but only traded it for a worse one: this
    // sidebar's own width is fixed (Main.qml), so a label wide enough to
    // want more room than the sidebar has just takes it from the combo box
    // next to it -- which is how "no margin left, and the label text still
    // ran past the sidebar's own edge" showed up for Russian. A label
    // column can borrow space from its neighbor; it can't invent space the
    // sidebar doesn't have.
    //
    // maximumWidth is the actual fix: it caps how far GridLayout will *let*
    // the column grow, guaranteeing the combo box column always keeps a
    // livable minimum regardless of translation length. elide is what
    // makes that cap safe to hit -- past it, the label truncates with an
    // ellipsis instead of overflowing the column GridLayout capped it at.
    //
    // Deliberately no Layout.fillWidth here -- that would make this label
    // compete with the combo box column for GridLayout's leftover space,
    // and once the label maxes out at 120 that offered share doesn't get
    // handed back to the combo box, it just goes unused -- the dead gap
    // (and lost right-edge margin) reported after the first version of
    // this fix. Left at its own natural (short-text) or capped
    // (long-text) size, every bit of leftover space in the row goes to the
    // combo box's own fillWidth instead, same as before this label column
    // needed a cap at all.
    component RowLabel: Label {
        Layout.maximumWidth: 120
        Layout.minimumWidth: 80
        elide: Text.ElideRight
    }

    ColumnLayout {
        id: column
        spacing: 8
        anchors.margins: 16
        width: parent.width - 32
        x: 16

        SectionHeading { text: qsTr("Port · PORT") }

        GridLayout {
            Layout.fillWidth: true
            enabled: !AppController.portOpen
            columns: 2
            columnSpacing: 10
            rowSpacing: 8

            RowLabel { text: qsTr("Port") }
            RowLayout {
                Layout.fillWidth: true

                PortComboBox {
                    id: portCombo
                    Layout.fillWidth: true
                    currentIndex: {
                        const byName = AppController.portListModel.indexOfPortName(AppController.selectedPortName)
                        return byName >= 0 ? byName : (count > 0 ? 0 : -1)
                    }
                    onActivated: AppController.selectedPortName = AppController.portListModel.portNameAt(currentIndex)
                }
                ToolButton {
                    icon.source: "qrc:/icons/refresh.svg"
                    icon.color: Theme.text
                    onClicked: AppController.portListModel.refresh()
                }
            }

            RowLabel { text: qsTr("Baud rate") }
            ComboBox {
                id: baudCombo
                Layout.fillWidth: true
                model: SerialOptions.baudRateOptions()
                currentIndex: 8 // 115200
            }

            RowLabel { text: qsTr("Data bits") }
            ComboBox {
                id: dataBitsCombo
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
                // Initial selection (and re-selection after a language
                // change) is handled by rebuildOptions().
            }

            RowLabel { text: qsTr("Parity") }
            ComboBox {
                id: parityCombo
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
            }

            RowLabel { text: qsTr("Stop bits") }
            ComboBox {
                id: stopBitsCombo
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
            }

            RowLabel { text: qsTr("Flow control") }
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
        CheckBox {
            text: qsTr("Append CRC (CRC16/MODBUS)")
            checked: AppController.crcEnabled
            onToggled: AppController.crcEnabled = checked
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
