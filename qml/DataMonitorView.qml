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
// arrive full of ANSI color escape codes, which LogListModel's per-line
// rendering turns into <span> runs (see ansi_text.h) -- those colors only
// read correctly against a dark backdrop, the same as any other terminal.
//
// The whole scrollback renders as ONE continuous rich-text document (a
// single read-only TextEdit) rather than a ListView of one row per entry:
// a ListView delegate only ever lets the user select within a single row
// at a time, and dragging a selection across multiple lines -- like any
// real terminal -- needs one shared text document to select across. The
// document is built incrementally (contentEdit.insert()/remove() from
// LogListModel's lineAppended()/lineEvicted()) rather than by re-binding
// `text` to the whole scrollback on every new line, which would re-layout
// potentially thousands of lines just to add one.
Item {
    id: root

    property bool paused: false
    property bool wrapLines: true

    // Strips each line's leading "HH:mm:ss  TAG  " prefix (see
    // LogListModel::lineHtml) from a chunk of selected plain text --
    // dragging a selection across several lines otherwise pulls each
    // line's timestamp and TX/RX/SYS/ERR tag along with it, which is noise
    // when what you actually want is just the payload. QQuickTextEdit has
    // no notion of an unselectable region (the whole document is one
    // uniform text area) and no rectangular/column-selection mode either,
    // so the drag highlight itself will still visually cover the prefix --
    // only what actually lands on the clipboard gets cleaned up here.
    //
    // Both gaps in the prefix, and any padding inside the tag itself
    // ("TX " -> "TX" + 1 pad char), render as U+00A0 rather than a plain
    // space: lineHtml() has to use &nbsp; there since a rich-text renderer
    // collapses consecutive literal HTML spaces, which would otherwise
    // throw off the fixed-width column alignment.
    //
    // The timestamp half is optional so this still matches correctly with
    // "Show timestamp" off. Only lines that actually START with this
    // pattern get stripped -- one that begins mid-line (the selection
    // started past the prefix already) is left untouched rather than
    // risking eating real content that happens to look similar.
    function stripLineColumns(text) {
        const prefix = /^(?:\d{2}:\d{2}:\d{2}  )?(?:(?:TX|RX)   |(?:SYS|ERR)  )/gm
        return text.replace(prefix, "")
    }

    // Copies the active selection (with its timestamp/tag columns
    // stripped) if there is one, otherwise the entire scrollback verbatim
    // -- a full-log copy keeps that context since it reads as an export,
    // not a value someone's about to paste elsewhere.
    function copyLog() {
        if (contentEdit.selectedText.length > 0) {
            clipboardHelper.text = root.stripLineColumns(contentEdit.selectedText)
            clipboardHelper.selectAll()
            clipboardHelper.copy()
            return
        }
        contentEdit.selectAll()
        contentEdit.copy()
        contentEdit.deselect()
    }

    // Off-screen helper: QML has no clipboard API of its own, but a hidden
    // TextEdit's copy() does the job without any C++ (same trick
    // RemoteAssistPanel.qml uses for "Copy code"). Needed only for the
    // stripped-selection path above -- contentEdit.copy() already talks to
    // the clipboard directly when copying it verbatim.
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

            Flickable {
                id: flick
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                contentWidth: root.wrapLines ? width : Math.max(width, contentEdit.paintedWidth)
                contentHeight: contentEdit.paintedHeight

                // Auto-scroll to the newest line unless the user paused or
                // has manually scrolled away from the bottom.
                property bool stickToBottom: true
                onContentHeightChanged: if (!root.paused && stickToBottom) contentY = Math.max(0, contentHeight - height)
                onContentYChanged: stickToBottom = (contentY + height >= contentHeight - 4)

                TextEdit {
                    id: contentEdit
                    width: root.wrapLines ? flick.width : implicitWidth
                    textFormat: TextEdit.RichText
                    font.family: Theme.monoFont
                    font.pixelSize: 12
                    color: Theme.consoleText
                    wrapMode: root.wrapLines ? TextEdit.Wrap : TextEdit.NoWrap

                    readOnly: true
                    selectByMouse: true
                    selectionColor: Theme.accent
                    selectedTextColor: Theme.background

                    function rebuild() { text = AppController.logModel.fullHtmlDump() }
                    Component.onCompleted: rebuild()

                    Connections {
                        target: AppController.logModel
                        function onLineAppended(html) { contentEdit.insert(contentEdit.length, html) }
                        function onLineEvicted(docLength) { contentEdit.remove(0, docLength) }
                        function onRebuildNeeded() { contentEdit.rebuild() }
                    }
                }
            }

            // Right-click only, declared above (so painted/hit-tested on
            // top of) the Flickable: acceptedButtons excluding the left
            // button means a left-press is never accepted here and falls
            // through to the TextEdit underneath for drag-selection (or to
            // the Flickable itself for a scrollbar-less drag-to-scroll),
            // while a right-press is caught here for the context menu.
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: contextMenu.popup()
            }
        }
    }
}
