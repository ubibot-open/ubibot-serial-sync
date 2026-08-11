import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
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

    // The context menu's "Save log" just opens Main.qml's existing
    // SaveLogDialog (same file-picker + AppController.saveLog() backend
    // as the toolbar/File-menu "Save log" action) rather than this view
    // building its own -- SaveLogDialog lives at the window level since
    // dialogs need Overlay.overlay to center on, not something this
    // panel has.
    signal saveLogRequested()

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

    // main.cpp forces the Fusion style app-wide, whose default Menu chrome
    // is a plain, boxy, light popup -- fine for the rest of the (light)
    // app, but it clashes badly sitting on top of this panel's dark
    // terminal. Menu's own `delegate` property only applies to model-
    // driven items, not ones declared directly as children the way the
    // three below are, so each gets its own look via these instead: a
    // flat, borderless, rounded-highlight row (closer to a modern browser's
    // right-click menu) rather than Fusion's raised/bordered one. Inline
    // components have to live at the document's top level (same reason
    // Main.qml's CompactToolButton does), hence declaring them here rather
    // than nested inside the Menu itself.
    component DarkMenuItem: MenuItem {
        id: darkItem
        implicitWidth: 200
        implicitHeight: 32
        // MenuItem already has an `icon` grouped property (source/color/
        // width/height) inherited from AbstractButton; IconImage is what
        // Qt's own built-in item delegates use internally to actually
        // recolor an icon via that `color` -- a plain Image bound to
        // icon.source wouldn't do that on its own, and every icon here is
        // a dark-stroked SVG drawn for the (light) toolbar elsewhere in
        // the app, not this dark menu.
        icon.width: 15
        icon.height: 15
        icon.color: darkItem.enabled ? Theme.consoleText : Theme.consoleMuted
        contentItem: RowLayout {
            spacing: 10
            IconImage {
                source: darkItem.icon.source
                color: darkItem.icon.color
                Layout.preferredWidth: darkItem.icon.width
                Layout.preferredHeight: darkItem.icon.height
                Layout.leftMargin: 14
            }
            Label {
                text: darkItem.text
                font.pixelSize: 13
                color: darkItem.enabled ? Theme.consoleText : Theme.consoleMuted
                verticalAlignment: Text.AlignVCenter
                Layout.rightMargin: 14
                Layout.fillWidth: true
            }
        }
        background: Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            radius: 5
            color: darkItem.highlighted ? Theme.accent : "transparent"
        }
    }

    component DarkMenuSeparator: MenuSeparator {
        implicitHeight: 3
        contentItem: Rectangle {
            implicitHeight: 3
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.consoleBorder
        }
    }

    Menu {
        id: contextMenu
        topPadding: 6
        bottomPadding: 6
        background: Rectangle {
            implicitWidth: 200
            color: Theme.consoleBackground
            border.color: Theme.consoleBorder
            border.width: 1
            radius: 8
        }

        DarkMenuItem {
            text: qsTr("Copy")
            icon.source: "qrc:/icons/copy.svg"
            onTriggered: root.copyLog()
        }
        DarkMenuSeparator {}
        DarkMenuItem {
            text: qsTr("Clear log")
            icon.source: "qrc:/icons/clear.svg"
            onTriggered: AppController.logModel.clear()
        }
        DarkMenuItem {
            text: qsTr("Save log…")
            icon.source: "qrc:/icons/save.svg"
            onTriggered: root.saveLogRequested()
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
