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

        // Was a "Port · PORT" picker (PortComboBox) here too, shared with
        // the "Serial" panel's own one -- this panel doesn't drive the port
        // connection at all anymore (see the row-click handler below), so
        // a port picker here just implied a connection this tab has
        // nothing to do with. Pick/open the port from the "Serial" panel;
        // this one is purely for finding a command's text.
        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search commands")
            // Fusion's default placeholder color barely shows up against
            // this field's dark-mode background -- see Main.qml's
            // inputField for the same fix.
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
                // Rises above sibling rows while being dragged so it draws
                // over them instead of underneath as it moves past.
                z: dragArea.drag.active ? 2 : 0
                color: dragArea.containsMouse ? Qt.rgba(0, 0, 0, 0.04) : "transparent"

                // Set on press, checked in onClicked -- distinguishes an
                // actual drag from a plain click, both of which end in a
                // MouseArea release. Without this, dropping a dragged row
                // would also fire the click behavior below (load it into
                // the send box) since the press and release both still
                // land inside this MouseArea's bounds (it moves with the
                // row it's dragging).
                property bool dragMoved: false
                // Row index captured when the drag started -- moveRow()'s
                // "from" argument on release. Not just `index` at drop time:
                // nothing reorders the model *during* the drag (see below),
                // so index stays exactly this value throughout the whole
                // gesture anyway, but capturing it explicitly reads clearer
                // than relying on that.
                property int dragStartIndex: -1

                // Reordering only touches the *currently displayed* set of
                // rows (see moveRow()'s doc comment) -- disabled while a
                // search filter is narrowing that set, so a drag can't
                // quietly drop every non-matching row to the bottom of the
                // saved order.
                readonly property bool reorderable: searchField.text.length === 0

                // Declared before the right-click MouseArea/context menu
                // below, so those take priority for their own button/area
                // over this one's left-click/drag handling.
                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    hoverEnabled: true
                    // Otherwise the ListView (a Flickable) steals the
                    // gesture as a flick/scroll the moment it exceeds the
                    // platform's drag threshold, cutting the reorder drag
                    // short.
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
                        // Purely visual while the drag is in progress --
                        // moving the underlying model live (once per row
                        // the drag passes over) sounds nicer but actually
                        // fights this same `drag.target` override: every
                        // moveRow() makes the ListView re-lay-out, which
                        // reasserts every delegate's position (including
                        // this dragged one) and stomps on the manual `y`
                        // drag.target had just set, snapping the drag back
                        // to wherever the view thinks index*height belongs
                        // *before* the next mouse-move event can re-drag it.
                        // Committing once on release avoids that fight.
                        if (drag.active) delegateRoot.dragMoved = true
                    }
                    onReleased: {
                        if (delegateRoot.dragMoved) {
                            // Rows are a fixed, uniform height with no
                            // spacing/section headers, so the dragged
                            // item's own final y position converts straight
                            // to a target row index -- no dependency on
                            // ListView.indexAt(), which needs the mapped
                            // point to land within an actual delegate's
                            // hit-test bounds and was unreliable right at a
                            // row's edge.
                            const targetIndex = Math.max(0, Math.min(listView.count - 1,
                                Math.round(delegateRoot.y / delegateRoot.height)))
                            if (targetIndex !== delegateRoot.dragStartIndex)
                                AppController.moveCommandRow(delegateRoot.dragStartIndex, targetIndex)
                        }
                        // Snap back into the position the ListView itself
                        // assigns this index -- rows are a fixed height with
                        // no spacing/section headers, so that's just
                        // index * height. Needed whether or not the drag
                        // actually moved anything: `index` itself may have
                        // changed (moveCommandRow above), or the item may
                        // just need releasing back to its unchanged spot.
                        delegateRoot.y = delegateRoot.index * delegateRoot.height
                    }
                    onClicked: {
                        if (delegateRoot.dragMoved) return
                        // The device command library is just a quick way to
                        // find a command's text, not to fire it at the
                        // port -- every row stages its text into the
                        // manual-send box for the user to review/edit and
                        // send themselves. Params-bearing commands go
                        // through the params panel first so the user can
                        // fill in <key> values before it lands in the box.
                        if (delegateRoot.hasParams) root.openParams(delegateRoot.index)
                        else AppController.loadCommandIntoDraft(delegateRoot.index)
                    }
                }

                // "My templates" rows only -- edit/delete used to be a
                // permanent pair of buttons on every custom row, which read
                // as cluttered; a right-click menu (same pattern as
                // DataMonitorView.qml's own context menu) keeps the row
                // itself identical to a bundled command until you actually
                // want to manage it.
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
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("New template")
                onClicked: templateDialog.openForNew()
            }
        }
    }

    // Add/edit form for "My templates" -- a plain {name, content} pair with
    // no protocol/params concept (see CommandListModel::addCustomTemplate).
    // Nested here rather than a separate file/registered top-level dialog
    // like ConnectionWizardDialog etc., since only this panel ever opens
    // it -- same reasoning as SettingsAboutDialog's own nested
    // updateResultDialog.
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

        // -1 means "creating a new template"; otherwise the row index
        // (captured when "Edit" was clicked) to update on save. Safe to
        // hold across the dialog's whole open/save lifetime since it's
        // modal -- nothing in this panel can reorder/refilter the list
        // while it has focus to invalidate that index.
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
}
