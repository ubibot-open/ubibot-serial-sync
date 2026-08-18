import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts
import UbiBot

// Right-hand "data monitor" pane: a scrolling, uncolored view of every
// TX/RX/SYS/ERR line, with a small header showing line count and byte
// counters. Pauses purely at the view level -- AppController.logModel keeps
// recording regardless, so unpausing shows everything that happened while
// paused.
//
// Follows AppController.themeMode like everything else (Theme.console*
// below) for its background/text/border, but every line now renders in the
// same plain Theme.consoleText color rather than being colored per-kind
// (TX/RX/SYS/ERR) or reproducing a device's own ANSI color codes. This used
// to be one continuous *rich-text* document, each line arriving as
// self-colored HTML, then briefly a plain-text document with a
// QSyntaxHighlighter (LogHighlighter) recoloring it live -- both added
// meaningful per-line memory overhead (rich-text spans, then per-block
// format ranges) that mattered once a device dumped a large stored log over
// serial. Plain, uncolored text is the cheapest a QTextDocument can be laid
// out and held in memory, and color coding wasn't essential to the data
// monitor's actual job of showing what came over the wire.
//
// The whole scrollback still renders as ONE continuous document (a single
// read-only TextEdit) rather than a ListView of one row per entry: a
// ListView delegate only ever lets the user select within a single row at
// a time, and dragging a selection across multiple lines -- like any real
// terminal -- needs one shared text document to select across. The
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
    // LogListModel::linePlainText) from a chunk of selected plain text --
    // dragging a selection across several lines otherwise pulls each
    // line's timestamp and TX/RX/SYS/ERR tag along with it, which is noise
    // when what you actually want is just the payload. QQuickTextEdit has
    // no notion of an unselectable region (the whole document is one
    // uniform text area) and no rectangular/column-selection mode either,
    // so the drag highlight itself will still visually cover the prefix --
    // only what actually lands on the clipboard gets cleaned up here.
    //
    // The timestamp half is optional so this still matches correctly with
    // "Show timestamp" off. Only lines that actually START with this
    // pattern get stripped -- one that begins mid-line (the selection
    // started past the prefix already) is left untouched rather than
    // risking eating real content that happens to look similar.
    function stripLineColumns(text) {
        const prefix = /^(?:\d{2}:\d{2}:\d{2}  )?(?:(?:TX|RX)   |(?:SYS|ERR)  )/gm
        return text.replace(prefix, "")
    }

    // String.arg() (QML's addition to JS strings, used by the header
    // counters below) formats a plain numeric argument using a general/
    // exponential-leaning conversion once it gets into the millions --
    // "1.00e+6" instead of "1000000" -- since it can't tell a genuinely
    // huge value from one that's just a big integer. Pre-converting to a
    // string ourselves (String() uses plain notation up to 1e21, far past
    // anything a byte/line counter will ever reach) sidesteps that, and
    // formatBytes below is also just more readable than a long run of raw
    // digits once a stream's been running a while.
    function formatCount(n) {
        return String(n)
    }
    function formatBytes(n) {
        if (n < 1024) return n + " B"
        if (n < 1024 * 1024) return (n / 1024).toFixed(1) + " KB"
        if (n < 1024 * 1024 * 1024) return (n / (1024 * 1024)).toFixed(1) + " MB"
        return (n / (1024 * 1024 * 1024)).toFixed(1) + " GB"
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
            spacing: 12
            IconImage {
                source: darkItem.icon.source
                color: darkItem.icon.color
                Layout.preferredWidth: darkItem.icon.width
                Layout.preferredHeight: darkItem.icon.height
                Layout.leftMargin: 16
            }
            Label {
                text: darkItem.text
                font.pixelSize: 13
                color: darkItem.enabled ? Theme.consoleText : Theme.consoleMuted
                verticalAlignment: Text.AlignVCenter
                Layout.rightMargin: 16
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
            anchors.leftMargin: 12
            anchors.rightMargin: 12
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
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                Label {
                    text: qsTr("Data monitor")
                    font.pixelSize: 11
                    font.letterSpacing: 1
                    color: Theme.consoleMuted
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: qsTr("%1 lines").arg(root.formatCount(AppController.logModel.totalLineCount))
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.consoleMuted
                }
                Label {
                    text: qsTr("Rx %1").arg(root.formatBytes(AppController.logModel.rxBytes))
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.consoleMuted
                }
                Label {
                    text: qsTr("Tx %1").arg(root.formatBytes(AppController.logModel.txBytes))
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
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                anchors.topMargin: 12
                anchors.bottomMargin: 12
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                contentWidth: root.wrapLines ? width : Math.max(width, contentEdit.paintedWidth)
                contentHeight: contentEdit.paintedHeight

                // Auto-scroll to the newest line unless the user paused or
                // has manually scrolled away from the bottom.
                property bool stickToBottom: true
                onContentHeightChanged: if (!root.paused && stickToBottom) contentY = Math.max(0, contentHeight - height)
                onContentYChanged: stickToBottom = (contentY + height >= contentHeight - 4)

                // A large data dump can run the scrollback into thousands of
                // lines -- the mouse wheel alone (the only way to move
                // around before this) makes finding a specific spot in that
                // tedious. AlwaysOn (rather than the default auto-hiding
                // overlay) so it's visible without having to hover first --
                // the whole point is making "how much is there / where am I"
                // obvious at a glance, not just adding a way to drag.
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    contentItem: Rectangle {
                        implicitWidth: 8
                        radius: width / 2
                        color: Theme.consoleMuted
                        opacity: parent.pressed ? 0.9 : (parent.hovered ? 0.7 : 0.45)
                    }
                    background: Rectangle {
                        implicitWidth: 8
                        color: Theme.consoleBorder
                        opacity: 0.3
                    }
                }
                // Only actually scrollable when wrapLines is off (contentWidth
                // matches the viewport exactly otherwise) -- explicitly
                // hidden rather than trusting policy: AsNeeded to notice
                // that on its own, since a stray sub-pixel rounding
                // difference between contentWidth and the viewport was
                // enough to show a spurious full-width one even while
                // wrapped. Same styling as the vertical one above for a
                // consistent look either way.
                ScrollBar.horizontal: ScrollBar {
                    policy: ScrollBar.AsNeeded
                    visible: !root.wrapLines
                    contentItem: Rectangle {
                        implicitHeight: 8
                        radius: height / 2
                        color: Theme.consoleMuted
                        opacity: parent.pressed ? 0.9 : (parent.hovered ? 0.7 : 0.45)
                    }
                    background: Rectangle {
                        implicitHeight: 8
                        color: Theme.consoleBorder
                        opacity: 0.3
                    }
                }

                TextEdit {
                    id: contentEdit
                    width: root.wrapLines ? flick.width : implicitWidth
                    textFormat: TextEdit.PlainText
                    font.family: AppController.logFontFamily
                    font.pixelSize: AppController.logFontSize
                    color: Theme.consoleText
                    wrapMode: root.wrapLines ? TextEdit.Wrap : TextEdit.NoWrap

                    readOnly: true
                    selectByMouse: true
                    selectionColor: Theme.accent
                    selectedTextColor: Theme.accentForeground

                    function rebuild() { text = AppController.logModel.fullPlainDump() }
                    Component.onCompleted: rebuild()

                    Connections {
                        target: AppController.logModel
                        function onLineAppended(text) { contentEdit.insert(contentEdit.length, text) }
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
