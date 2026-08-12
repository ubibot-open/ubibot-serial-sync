import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Three-step "connect a new device" flow: pick a detected serial port, pick
// the UbiBot model it is, confirm & finish. On finish, AppController opens
// the port at 115200 8-N-1 with that model's command set loaded.
Dialog {
    id: root
    title: qsTr("Connection wizard")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 560
    height: 460

    property int selectedPortRow: -1
    property string selectedModelId: ""
    property string errorText: ""

    onOpened: {
        AppController.portListModel.refresh()
        selectedPortRow = AppController.portListModel.rowCount > 0 ? 0 : -1
        selectedModelId = AppController.modelIds.length > 0 ? AppController.modelIds[0] : ""
        errorText = ""
        swipe.setCurrentIndex(0)
    }

    contentItem: ColumnLayout {
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 0
            Repeater {
                model: [qsTr("1. Select a serial port"), qsTr("2. Select a device model"), qsTr("3. Finish")]
                delegate: Rectangle {
                    required property string modelData
                    required property int index
                    Layout.fillWidth: true
                    height: 28
                    border.color: Theme.divider
                    border.width: 1
                    color: swipe.currentIndex === index ? Theme.accent : "transparent"
                    Label {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 12
                        color: swipe.currentIndex === index ? Theme.background : Theme.text
                    }
                }
            }
        }

        SwipeView {
            id: swipe
            Layout.fillWidth: true
            Layout.fillHeight: true
            interactive: false
            // SwipeView does not clip its pages by default -- without this,
            // the not-yet-current page (e.g. the model list) lays out right
            // next to the current one inside the internal ListView and
            // bleeds out past the dialog's edge instead of staying hidden.
            clip: true

            // --- Page 1: port ---------------------------------------------
            ListView {
                model: AppController.portListModel
                clip: true
                delegate: ItemDelegate {
                    id: portDelegate
                    required property int index
                    required property string displayLabel
                    required property bool recommended
                    width: ListView.view.width
                    // ItemDelegate's own implicit height comes from its
                    // (unused) text/icon content, not from the RowLayout
                    // stuffed into it below -- left alone, rows collapse to
                    // that unrelated default and the label gets cramped
                    // against the delegate's edges. Size off the row's own
                    // implicit height instead so the padding is consistent.
                    implicitHeight: portRow.implicitHeight + 20
                    highlighted: root.selectedPortRow === index
                    onClicked: root.selectedPortRow = index

                    RowLayout {
                        id: portRow
                        anchors.fill: parent
                        anchors.margins: 10
                        Label { text: portDelegate.displayLabel; Layout.fillWidth: true }
                        Label {
                            visible: portDelegate.recommended
                            text: qsTr("Recommended")
                            color: Theme.accent
                            font.pixelSize: 11
                        }
                    }
                }
            }

            // --- Page 2: model ----------------------------------------------
            ListView {
                model: AppController.modelIds
                clip: true
                delegate: ItemDelegate {
                    id: modelDelegate
                    required property string modelData
                    required property int index
                    width: ListView.view.width
                    // Two lines (model name + wrapped description) need more
                    // room than ItemDelegate's own default implicit height
                    // gives them -- size off the ColumnLayout's implicit
                    // height instead, same reasoning as the port list above.
                    implicitHeight: modelColumn.implicitHeight + 20
                    highlighted: root.selectedModelId === modelData
                    onClicked: root.selectedModelId = modelData

                    ColumnLayout {
                        id: modelColumn
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 4
                        Label { text: modelDelegate.modelData; font.bold: true }
                        Label {
                            text: AppController.modelDescriptionFor(modelDelegate.modelData)
                            font.pixelSize: 11
                            color: Theme.textMuted
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // --- Page 3: summary ----------------------------------------------
            ColumnLayout {
                spacing: 10
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    // Was a hardcoded "115200 8-N-1" -- now reflects
                    // whatever this model will actually open at, since a
                    // JSON-protocol model can carry its own serial defaults
                    // (docs/device-json-protocol-schema.md#5) instead of
                    // the historical fixed rate every AT device used.
                    text: qsTr("Ready to open %1 at %2 and load the %3 command set.")
                        .arg(root.selectedPortRow >= 0 ? AppController.portListModel.portNameAt(root.selectedPortRow) : "")
                        .arg(AppController.serialSummaryForModel(root.selectedModelId))
                        .arg(root.selectedModelId)
                }
                Label {
                    visible: root.errorText.length > 0
                    text: root.errorText
                    color: Theme.error
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                Item { Layout.fillHeight: true }
            }
        }
    }

    footer: DialogButtonBox {
        Button {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
        Button {
            text: qsTr("Back")
            enabled: swipe.currentIndex > 0
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
            onClicked: swipe.setCurrentIndex(swipe.currentIndex - 1)
        }
        Button {
            text: swipe.currentIndex === 2 ? qsTr("Finish") : qsTr("Next")
            enabled: swipe.currentIndex === 0 ? root.selectedPortRow >= 0
                     : swipe.currentIndex === 1 ? root.selectedModelId.length > 0
                     : true
            highlighted: true
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
            onClicked: {
                if (swipe.currentIndex < 2) {
                    swipe.setCurrentIndex(swipe.currentIndex + 1)
                    return
                }
                const portName = AppController.portListModel.portNameAt(root.selectedPortRow)
                const error = AppController.finishWizard(portName, root.selectedModelId)
                if (error.length > 0) root.errorText = error
                else root.accept()
            }
        }
    }
}
