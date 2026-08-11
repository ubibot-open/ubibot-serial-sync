import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Right-hand "data monitor" pane: a scrolling, color-coded view of every
// TX/RX/SYS/ERR line, with a small header showing line count and byte
// counters. Pauses purely at the view level -- AppController.logModel keeps
// recording regardless, so unpausing shows everything that happened while
// paused.
//
// Styled as a dark terminal rather than a tinted panel on the app's light
// background: real hardware logs (RT-Thread/msh boot output and similar)
// arrive full of ANSI color escape codes, which LogListModel's `html` role
// turns into <span> runs (see ansi_text.h) -- those colors only read
// correctly against a dark backdrop, the same as any other terminal emulator.
Item {
    id: root

    property bool paused: false
    property bool wrapLines: true

    // Each visible row's content item is independently selectable
    // (selectByMouse on the TextEdit below); this walks the currently-
    // instantiated delegates for whichever one(s) hold a non-empty
    // selection rather than tracking it via bindings, since ListView only
    // ever instantiates visible (+ a small cache buffer of) rows anyway,
    // and a binding updated from every row's onSelectedTextChanged can't
    // tell "this row's selection was cleared" apart from "a different
    // row's selection just started" via the value alone.
    function selectedLogText() {
        var parts = []
        for (var i = 0; i < listView.contentItem.children.length; ++i) {
            var row = listView.contentItem.children[i]
            if (row && row.contentLabel && row.contentLabel.selectedText.length > 0)
                parts.push(row.contentLabel.selectedText)
        }
        return parts.join("\n")
    }

    // Copies the active selection if there is one, otherwise the entire
    // scrollback -- what the context menu's "Copy" is for either way, so
    // both it and a bare Ctrl+C land here.
    function copyLog() {
        var selected = root.selectedLogText()
        clipboardHelper.text = selected.length > 0 ? selected : AppController.logModel.plainTextDump()
        clipboardHelper.selectAll()
        clipboardHelper.copy()
    }

    // Off-screen helper: QML has no clipboard API of its own, but a hidden
    // TextEdit's copy() does the job without any C++ (same trick
    // RemoteAssistPanel.qml uses for "Copy code").
    TextEdit { id: clipboardHelper; visible: false }

    Menu {
        id: contextMenu
        MenuItem {
            text: qsTr("Copy")
            onTriggered: root.copyLog()
        }
        MenuItem {
            text: qsTr("Clear log")
            onTriggered: AppController.logModel.clear()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            color: Theme.consoleBackground
            border.width: 0
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.consoleBorder }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14

                Label {
                    text: qsTr("Data monitor")
                    font.pixelSize: 11
                    font.letterSpacing: 1
                    color: Theme.consoleMuted
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: qsTr("%1 lines").arg(AppController.logModel.lineCount)
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.consoleMuted
                }
                Label {
                    text: qsTr("Rx %1 B").arg(AppController.logModel.rxBytes)
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.consoleMuted
                }
                Label {
                    text: qsTr("Tx %1 B").arg(AppController.logModel.txBytes)
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.consoleMuted
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.consoleBackground

            ListView {
                id: listView
                anchors.fill: parent
                leftMargin: 14
                rightMargin: 14
                topMargin: 10
                bottomMargin: 10
                clip: true
                model: AppController.logModel
                spacing: 3
                boundsBehavior: Flickable.StopAtBounds

                // Auto-scroll to the newest line unless the user paused or
                // has manually scrolled away from the bottom.
                property bool stickToBottom: true
                onCountChanged: if (!root.paused && stickToBottom) positionViewAtEnd()
                onContentYChanged: {
                    stickToBottom = (contentY + height >= contentHeight - 4)
                }

                delegate: RowLayout {
                    id: rowDelegate
                    width: listView.width - listView.leftMargin - listView.rightMargin
                    spacing: 10

                    required property string time
                    required property string dir
                    required property string html
                    required property string color

                    // Exposed so selectedLogText() above can reach into
                    // whichever rows are currently instantiated.
                    property alias contentLabel: contentLabelId

                    Label {
                        text: rowDelegate.time
                        color: Theme.consoleMuted
                        font.family: Theme.monoFont
                        font.pixelSize: 12
                        visible: text.length > 0
                    }
                    Label {
                        text: rowDelegate.dir
                        color: rowDelegate.color
                        font.family: Theme.monoFont
                        font.pixelSize: 12
                        Layout.preferredWidth: 30
                    }
                    // Read-only TextEdit rather than Label/Text: neither
                    // Label (a Control) nor plain Text expose
                    // selectByMouse/selectionColor/selectedTextColor in
                    // this Qt version -- those are TextEdit/TextInput-only
                    // -- and mouse-selectable content is the whole point.
                    TextEdit {
                        id: contentLabelId
                        text: rowDelegate.html
                        textFormat: TextEdit.RichText
                        color: rowDelegate.color
                        font.family: Theme.monoFont
                        font.pixelSize: 12
                        wrapMode: root.wrapLines ? TextEdit.Wrap : TextEdit.NoWrap
                        Layout.fillWidth: true

                        readOnly: true
                        selectByMouse: true
                        selectionColor: Theme.accent
                        selectedTextColor: Theme.background
                    }
                }
            }

            // Right-click only, declared above (so painted/hit-tested on
            // top of) the ListView: acceptedButtons excluding the left
            // button means a left-press is never accepted here and falls
            // through to the row Labels underneath for drag-selection,
            // while a right-press is caught here for the context menu.
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: contextMenu.popup()
            }
        }
    }
}
