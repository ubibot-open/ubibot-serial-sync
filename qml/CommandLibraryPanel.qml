import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Left-hand panel for "Device commands" mode: model picker, search, favorite
// /group filter chips, and the grouped, scrollable command list itself.
Item {
    id: root

    // Emitted instead of calling AppController directly when a command needs
    // parameters, since the params panel lives in Main.qml, not here.
    signal openParams(int row)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Label { text: qsTr("Device model") }

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
            font.pixelSize: 12
            color: Theme.textMuted
        }

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search commands")
            text: AppController.commandModel.searchText
            onTextChanged: AppController.commandModel.searchText = text
        }

        Flow {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: AppController.commandModel.filterChips
                delegate: Button {
                    required property var modelData
                    text: modelData.label
                    checkable: true
                    checked: modelData.checked
                    onClicked: AppController.commandModel.filterKey = modelData.key
                }
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: AppController.commandModel

            section.property: "group"
            section.criteria: ViewSection.FullString
            section.delegate: Rectangle {
                width: listView.width
                height: 24
                color: Qt.rgba(0, 0, 0, 0.04)
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: section.toUpperCase()
                    font.pixelSize: 10
                    font.letterSpacing: 1
                    color: Theme.accent700
                }
            }

            delegate: Rectangle {
                id: delegateRoot
                required property int index
                required property string name
                required property string cmd
                required property bool hasParams
                required property bool favorite

                width: listView.width
                height: 48
                color: rowMouse.containsMouse ? Qt.rgba(0, 0, 0, 0.04) : "transparent"

                // Declared before the row content below, so it paints
                // underneath it: the star's own MouseArea (nested inside the
                // RowLayout, declared later/on top) hit-tests first and
                // consumes clicks aimed at the star, leaving this one to
                // handle clicks anywhere else on the row.
                MouseArea {
                    id: rowMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (delegateRoot.hasParams) root.openParams(delegateRoot.index)
                        else AppController.activateCommandRow(delegateRoot.index)
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 9

                    Label {
                        id: starLabel
                        text: "★"
                        color: delegateRoot.favorite ? Theme.accent : "#b7b7ba"
                        font.pixelSize: 14

                        MouseArea {
                            anchors.fill: parent
                            anchors.margins: -6
                            onClicked: AppController.toggleFavorite(delegateRoot.index)
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1
                        Label { text: delegateRoot.name; font.bold: true }
                        Label {
                            text: delegateRoot.cmd
                            font.family: Theme.monoFont
                            font.pixelSize: 11
                            color: Theme.accent700
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
