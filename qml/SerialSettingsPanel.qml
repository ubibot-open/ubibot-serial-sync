// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

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
    property bool wrapLines: true

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

    function applyModelSerialDefaults() {
        const d = AppController.serialDefaultsForModel(AppController.currentModelId)
        if (!d.present) return
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
        applyModelSerialDefaults()
    }

    component SectionHeading: Label {
        Layout.fillWidth: true
        Layout.topMargin: 8
        font.pixelSize: Theme.baseFontSize - 1
        font.letterSpacing: 1
        color: Theme.textMuted
    }
    component RowLabel: Label {
        Layout.maximumWidth: 120
        Layout.minimumWidth: 80
        elide: Text.ElideRight
    }
    component FitCheckBox: CheckBox {
        Layout.fillWidth: true
        contentItem: Text {
            text: parent.text
            font: parent.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            leftPadding: parent.indicator.width + parent.spacing
            color: Theme.text
        }
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
                    currentIndex: AppController.portListModel.indexOfPortName(AppController.selectedPortName)
                    onActivated: AppController.selectedPortName = AppController.portListModel.portNameAt(currentIndex)
                }
                ToolButton {
                    icon.source: "qrc:/icons/refresh.svg"
                    icon.color: Theme.text
                    onClicked: AppController.refreshPorts()
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
        Button {
            id: openPortButton
            text: AppController.portOpen ? qsTr("Close port") : qsTr("Open port")
            highlighted: true
            Layout.fillWidth: true
            Layout.topMargin: 4
            Layout.preferredHeight: 44
            background: Rectangle {
                radius: 3
                readonly property color base: AppController.portOpen ? Theme.error : Theme.accent
                color: openPortButton.pressed || openPortButton.hovered
                       ? (AppController.portOpen ? Qt.darker(base, 1.2) : Theme.accent700)
                       : base
            }
            contentItem: Text {
                text: openPortButton.text
                font: openPortButton.font
                color: Theme.accentForeground
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                if (AppController.portOpen) {
                    AppController.closePort()
                } else {
                    AppController.openPort(root.selectedPort, root.selectedBaud,
                                            root.selectedDataBits, root.selectedParity,
                                            root.selectedStopBits, root.selectedFlowControl)
                }
            }
        }

        SectionHeading { text: qsTr("Receive · RECEIVE") }

        RowLayout {
            RadioButton { text: "ASCII"; checked: !AppController.logModel.hexMode; onToggled: if (checked) AppController.logModel.hexMode = false }
            RadioButton { text: "HEX"; checked: AppController.logModel.hexMode; onToggled: if (checked) AppController.logModel.hexMode = true }
        }
        FitCheckBox {
            text: qsTr("Show timestamp")
            checked: AppController.logModel.showTimestamp
            onToggled: AppController.logModel.showTimestamp = checked
        }
        FitCheckBox {
            text: qsTr("Wrap lines")
            checked: root.wrapLines
            onToggled: root.wrapLines = checked
        }
        FitCheckBox {
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
        FitCheckBox {
            text: qsTr("Append CRC (CRC16/MODBUS)")
            checked: AppController.crcEnabled
            onToggled: AppController.crcEnabled = checked
        }
        RowLayout {
            FitCheckBox {
                text: qsTr("Repeat send")
                checked: AppController.repeatSendEnabled
                onToggled: AppController.repeatSendEnabled = checked
            }
            SpinBox {
                from: 0
                to: 3600000
                stepSize: 50
                editable: true
                value: AppController.repeatIntervalMs
                onValueModified: AppController.repeatIntervalMs = value
            }
            Label { text: qsTr("ms") }
        }
    }
}
