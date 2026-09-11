// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Left-hand panel for "Device commands" mode: model picker, search, and the
// flat, scrollable command list itself (plus the user's own "My templates",
// merged into the same list -- see CommandListModel). No grouping/
// favorites/filter chips -- removed per user feedback that a handful of
// commands per model didn't need organizing; CommandListModel sorts
// whichever row was clicked most recently to the top instead.
Item {
    id: root

    // Emitted instead of calling AppController directly when a command needs
    // parameters, since the params panel lives in Main.qml, not here.
    signal openParams(int row)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label { text: qsTr("Device model · Model") }

        ComboBox {
            id: modelCombo
            Layout.fillWidth: true
            model: AppController.modelIds
            currentIndex: model.indexOf(AppController.currentModelId)
            onActivated: AppController.currentModelId = currentText
        }

        Label {
            Layout.fillWidth: true
            text: AppController.currentModelDescription
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.baseFontSize
            color: Theme.textMuted
        }

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search commands")
            placeholderTextColor: Theme.textMuted
            text: AppController.commandModel.searchText
            onTextChanged: AppController.commandModel.searchText = text
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Drag a row to reorder")
            font.pixelSize: Theme.baseFontSize - 1
            color: Theme.textMuted
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: AppController.commandModel

            delegate: Rectangle {
                id: delegateRoot
                required property int index
                required property string name
                required property string cmd
                required property bool hasParams
                required property bool isCustom

                width: listView.width
                height: 56
                z: dragArea.drag.active ? 2 : 0
                color: dragArea.containsMouse ? Qt.rgba(0, 0, 0, 0.04) : "transparent"
                property bool dragMoved: false
                property int dragStartIndex: -1
                readonly property bool reorderable: searchField.text.length === 0
                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    hoverEnabled: true
                    preventStealing: true
                    drag.target: delegateRoot.reorderable ? delegateRoot : null
                    drag.axis: Drag.YAxis
                    drag.minimumY: 0
                    drag.maximumY: Math.max(0, listView.contentHeight - delegateRoot.height)

                    onPressed: {
                        delegateRoot.dragMoved = false
                        delegateRoot.dragStartIndex = delegateRoot.index
                    }
                    onPositionChanged: (mouse) => {
                        if (drag.active) delegateRoot.dragMoved = true
                    }
                    onReleased: {
                        if (delegateRoot.dragMoved) {
                            const targetIndex = Math.max(0, Math.min(listView.count - 1,
                                Math.round(delegateRoot.y / delegateRoot.height)))
                            if (targetIndex !== delegateRoot.dragStartIndex)
                                AppController.moveCommandRow(delegateRoot.dragStartIndex, targetIndex)
                        }
                        delegateRoot.y = delegateRoot.index * delegateRoot.height
                    }
                    onClicked: {
                        if (delegateRoot.dragMoved) return
                        if (delegateRoot.hasParams) root.openParams(delegateRoot.index)
                        else AppController.loadCommandIntoDraft(delegateRoot.index)
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    enabled: delegateRoot.isCustom
                    onClicked: contextMenu.popup()
                }

                Menu {
                    id: contextMenu
                    MenuItem {
                        text: qsTr("Edit")
                        onTriggered: templateDialog.openForEdit(delegateRoot.index, delegateRoot.name, delegateRoot.cmd)
                    }
                    MenuItem {
                        text: qsTr("Delete")
                        onTriggered: AppController.removeCustomTemplate(delegateRoot.index)
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    anchors.topMargin: 10
                    anchors.bottomMargin: 10
                    spacing: 11

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1
                        Label { text: delegateRoot.name }
                        Label {
                            text: delegateRoot.cmd
                            font.family: Theme.monoFont
                            font.pixelSize: Theme.baseFontSize - 1
                            color: Theme.accent700
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Button {
                text: qsTr("Batch commands")
                onClicked: batchListDialog.open()
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("New template")
                onClicked: templateDialog.openForNew()
            }
        }
    }

    Dialog {
        id: templateDialog
        title: editRow >= 0 ? qsTr("Edit template") : qsTr("New template")
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 380
        palette: Theme.palette
        font.family: Theme.baseFontFamily
        font.pixelSize: Theme.baseFontSize
        background: DialogCard {}
        Overlay.modal: ModalDim {}
        header: Label {
            text: templateDialog.title
            font.bold: true
            padding: 14
            color: Theme.text
        }
        footer: DialogButtonBox {
            palette: Theme.palette
            Button {
                text: qsTr("Cancel")
                palette: Theme.palette
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
            Button {
                text: qsTr("Save")
                palette: Theme.palette
                enabled: nameField.text.trim().length > 0 && contentField.text.trim().length > 0
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                onClicked: {
                    if (templateDialog.editRow >= 0)
                        AppController.updateCustomTemplate(templateDialog.editRow, nameField.text, contentField.text)
                    else
                        AppController.addCustomTemplate(nameField.text, contentField.text)
                    templateDialog.close()
                }
            }
        }
        property int editRow: -1

        function openForNew() {
            editRow = -1
            nameField.text = ""
            contentField.text = ""
            open()
        }
        function openForEdit(row, name, content) {
            editRow = row
            nameField.text = name
            contentField.text = content
            open()
        }

        contentItem: ColumnLayout {
            spacing: 10

            Label { text: qsTr("Name") }
            TextField {
                id: nameField
                Layout.fillWidth: true
                placeholderText: qsTr("e.g. Reset device")
                placeholderTextColor: Theme.textMuted
            }

            Label { text: qsTr("Content") }
            TextArea {
                id: contentField
                Layout.fillWidth: true
                Layout.preferredHeight: 90
                placeholderText: qsTr("The literal text to send")
                placeholderTextColor: Theme.textMuted
                wrapMode: TextArea.Wrap
                font.family: Theme.monoFont
                background: Rectangle { color: Theme.surface; border.color: Theme.divider; border.width: 1 }
            }
        }
    }

    Dialog {
        id: batchListDialog
        title: qsTr("Batch commands")
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 420
        height: 460
        palette: Theme.palette
        font.family: Theme.baseFontFamily
        font.pixelSize: Theme.baseFontSize
        background: DialogCard {}
        Overlay.modal: ModalDim {}
        header: Label {
            text: batchListDialog.title
            font.bold: true
            padding: 14
            color: Theme.text
        }
        footer: DialogButtonBox {
            palette: Theme.palette
            Button {
                text: qsTr("Close")
                palette: Theme.palette
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
        }

        contentItem: ColumnLayout {
            spacing: 10

            Label {
                Layout.fillWidth: true
                visible: batchListView.count === 0
                text: qsTr("No batch commands yet")
                color: Theme.textMuted
                wrapMode: Text.WordWrap
            }

            ListView {
                id: batchListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: AppController.batchCommandModel

                delegate: Rectangle {
                    id: batchDelegate
                    required property int index
                    required property string name
                    required property int stepCount
                    required property int intervalMs
                    readonly property bool running: batchDelegate.index === AppController.runningBatchRow

                    width: batchListView.width
                    height: 56
                    color: running ? Theme.accentTint
                        : (rowHover.containsMouse ? Qt.rgba(0, 0, 0, 0.04) : "transparent")
                    border.color: Theme.divider
                    border.width: 1

                    MouseArea {
                        id: rowHover
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            if (batchDelegate.running) AppController.stopBatchCommand()
                            else AppController.startBatchCommand(batchDelegate.index)
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.RightButton
                        onClicked: batchContextMenu.popup()
                    }
                    Menu {
                        id: batchContextMenu
                        MenuItem {
                            text: qsTr("Edit")
                            onTriggered: batchEditDialog.openForEdit(batchDelegate.index, batchDelegate.name,
                                                                      batchDelegate.intervalMs)
                        }
                        MenuItem {
                            text: qsTr("Delete")
                            onTriggered: AppController.removeBatchCommand(batchDelegate.index)
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 11

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1
                            Label { text: batchDelegate.name }
                            Label {
                                text: batchDelegate.running
                                    ? AppController.batchProgressText
                                    : qsTr("%1 steps · %2 ms interval").arg(batchDelegate.stepCount).arg(batchDelegate.intervalMs)
                                font.pixelSize: Theme.baseFontSize - 1
                                color: batchDelegate.running ? Theme.accent700 : Theme.textMuted
                            }
                        }
                        Label {
                            text: batchDelegate.running ? qsTr("■ Stop") : qsTr("▶ Run")
                            color: batchDelegate.running ? Theme.error : Theme.accent
                            font.bold: true
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("New batch")
                    onClicked: batchEditDialog.openForNew()
                }
            }
        }
    }

    Dialog {
        id: batchEditDialog
        title: editRow >= 0 ? qsTr("Edit batch command") : qsTr("New batch command")
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 460
        height: 520
        palette: Theme.palette
        font.family: Theme.baseFontFamily
        font.pixelSize: Theme.baseFontSize
        background: DialogCard {}
        Overlay.modal: ModalDim {}
        header: Label {
            text: batchEditDialog.title
            font.bold: true
            padding: 14
            color: Theme.text
        }
        footer: DialogButtonBox {
            palette: Theme.palette
            Button {
                text: qsTr("Cancel")
                palette: Theme.palette
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
            Button {
                text: qsTr("Save")
                palette: Theme.palette
                enabled: batchNameField.text.trim().length > 0 && stepsModel.count > 0
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                onClicked: {
                    const steps = batchEditDialog.collectSteps()
                    // Every step's text is still blank -- addBatchCommand/
                    // updateBatchCommand would silently no-op on this (same
                    // "ignore rather than store a useless row" rule as
                    // addCustomTemplate), which would otherwise look like a
                    // broken Save button. Leaving the dialog open instead at
                    // least gives the user something to notice and fix.
                    if (!steps.some(function (s) { return s.text.trim().length > 0 })) return
                    if (batchEditDialog.editRow >= 0)
                        AppController.updateBatchCommand(batchEditDialog.editRow, batchNameField.text, intervalSpin.value, steps)
                    else
                        AppController.addBatchCommand(batchNameField.text, intervalSpin.value, steps)
                    batchEditDialog.close()
                }
            }
        }

        // -1 means "creating a new batch"; otherwise the row index
        // (captured when "Edit" was clicked) to update on save -- same
        // convention as templateDialog.editRow above.
        property int editRow: -1

        ListModel { id: stepsModel }

        function openForNew() {
            editRow = -1
            batchNameField.text = ""
            intervalSpin.value = 500
            stepsModel.clear()
            addStep()
            open()
        }
        function openForEdit(row, name, intervalMs) {
            editRow = row
            batchNameField.text = name
            intervalSpin.value = intervalMs
            stepsModel.clear()
            const steps = AppController.stepsForBatchRow(row)
            for (var i = 0; i < steps.length; ++i)
                stepsModel.append({stepText: steps[i].text, isHex: steps[i].isHex, crc: steps[i].crc})
            if (stepsModel.count === 0) addStep()
            open()
        }
        function addStep() {
            stepsModel.append({stepText: "", isHex: false, crc: false})
        }
        function collectSteps() {
            const result = []
            for (var i = 0; i < stepsModel.count; ++i) {
                const item = stepsModel.get(i)
                result.push({text: item.stepText, isHex: item.isHex, crc: item.crc})
            }
            return result
        }

        contentItem: ColumnLayout {
            spacing: 10

            Label { text: qsTr("Name") }
            TextField {
                id: batchNameField
                Layout.fillWidth: true
                placeholderText: qsTr("e.g. Startup sequence")
                placeholderTextColor: Theme.textMuted
            }

            RowLayout {
                Label { text: qsTr("Interval") }
                SpinBox {
                    id: intervalSpin
                    from: 0
                    to: 3600000
                    stepSize: 50
                    editable: true
                }
                Label { text: qsTr("ms") }
                Item { Layout.fillWidth: true }
            }

            Label { text: qsTr("Steps") }

            ListView {
                id: stepsView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 6
                model: stepsModel

                delegate: Rectangle {
                    id: stepDelegate
                    required property int index
                    required property string stepText
                    required property bool isHex
                    required property bool crc

                    width: stepsView.width
                    height: 64
                    color: Theme.surface
                    border.color: Theme.divider
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 8

                        Label { text: (stepDelegate.index + 1) + "."; color: Theme.textMuted }

                        TextField {
                            Layout.fillWidth: true
                            text: stepDelegate.stepText
                            placeholderText: qsTr("Command text")
                            placeholderTextColor: Theme.textMuted
                            font.family: Theme.monoFont
                            onTextEdited: stepsModel.setProperty(stepDelegate.index, "stepText", text)
                        }

                        RadioButton {
                            text: "ASCII"
                            checked: !stepDelegate.isHex
                            onToggled: if (checked) stepsModel.setProperty(stepDelegate.index, "isHex", false)
                        }
                        RadioButton {
                            text: "HEX"
                            checked: stepDelegate.isHex
                            onToggled: if (checked) stepsModel.setProperty(stepDelegate.index, "isHex", true)
                        }
                        CheckBox {
                            text: qsTr("CRC")
                            checked: stepDelegate.crc
                            onToggled: stepsModel.setProperty(stepDelegate.index, "crc", checked)
                        }
                        ToolButton {
                            text: "✕"
                            enabled: stepsModel.count > 1
                            onClicked: stepsModel.remove(stepDelegate.index)
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Add step")
                    onClicked: batchEditDialog.addStep()
                }
            }
        }
    }
}
