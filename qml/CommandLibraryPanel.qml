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
        anchors.margins: 14
        spacing: 10

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
            font.pixelSize: 12
            color: Theme.textMuted
        }

        Label { text: qsTr("Port · PORT") }

        ComboBox {
            id: portCombo
            Layout.fillWidth: true
            textRole: "displayLabel"
            model: AppController.portListModel
            // Shared with the port picker on the "Serial" panel -- whichever
            // one the user picks a port in, both should agree on it (and
            // it's what "Open port" in the toolbar actually opens). Falls
            // back to the first detected port when nothing's been picked
            // yet or the previously-picked port is no longer present.
            currentIndex: {
                const byName = AppController.portListModel.indexOfPortName(AppController.selectedPortName)
                return byName >= 0 ? byName : (count > 0 ? 0 : -1)
            }
            onActivated: AppController.selectedPortName = AppController.portListModel.portNameAt(currentIndex)
            // Re-scan available ports right as the dropdown opens, so a
            // device plugged in after the app started shows up here without
            // having to switch to the Serial tab's separate refresh button.
            onPressedChanged: if (pressed) AppController.portListModel.refresh()
        }

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search commands")
            text: AppController.commandModel.searchText
            onTextChanged: AppController.commandModel.searchText = text
        }

        // Design renders these as flat, square-cornered chips (24px tall,
        // 12px label) rather than standard buttons -- hand-rolled rather
        // than Button since checkable Button pulls in the style's full
        // pill-shaped chrome and padding. The checked chip already stands
        // out via its inverted accent background, so the label itself
        // stays normal weight rather than every chip being bold regardless
        // of state.
        Flow {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: AppController.commandModel.filterChips
                delegate: Rectangle {
                    id: chip
                    required property var modelData
                    height: 24
                    width: chipLabel.implicitWidth + 18
                    color: modelData.checked ? Theme.accent : "transparent"
                    border.color: modelData.checked ? Theme.accent : Theme.divider
                    border.width: 1

                    Label {
                        id: chipLabel
                        anchors.centerIn: parent
                        text: chip.modelData.label
                        font.pixelSize: 12
                        color: chip.modelData.checked ? Theme.background : Theme.text
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: AppController.commandModel.filterKey = chip.modelData.key
                    }
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
                height: 29
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
                height: 56
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
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    anchors.topMargin: 8
                    anchors.bottomMargin: 8
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
                        // Bold only for favorited commands -- with every
                        // row's name bold regardless, the whole list read as
                        // uniformly heavy with no real hierarchy; this way
                        // bold actually signals something (starred).
                        Label { text: delegateRoot.name; font.bold: delegateRoot.favorite }
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
