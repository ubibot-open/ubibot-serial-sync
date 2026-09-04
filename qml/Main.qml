import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Window
import UbiBot

ApplicationWindow {
    id: window

    // Space around the visible UI reserved for the soft drop shadow (see
    // `frame` below) to bleed into -- the window's own true OS-level bounds
    // are this much bigger than what actually looks like "the window" on
    // screen in every direction. Collapses to 0 while maximized (no room
    // for a shadow then, and none is wanted -- a maximized window doesn't
    // have one natively either).
    readonly property int shadowMargin: 18
    readonly property bool shadowActive: visibility !== Window.Maximized
    readonly property int activeMargin: shadowActive ? shadowMargin : 0

    // Bumped from 1200x800 -- this session's own control-size/font-size/
    // spacing increases (bigger buttons, +2px spacing tokens, a larger
    // default base font, ...) grew the toolbar row's natural content width
    // past what 1200 has room for, clipping its right-aligned buttons off
    // the edge at that size (true when the "Open port" button still lived
    // here too, before it moved into the Port sidebar -- see
    // SerialSettingsPanel.qml). A modest bump gives that room back without
    // the window opening noticeably larger on a normal desktop. The extra
    // `+ shadowMargin * 2` on every size below is new -- these four numbers
    // used to describe the visible UI's own size directly, back when the
    // window's true OS-level bounds and the visible UI's bounds were the
    // same rectangle; now that `frame` (see below) sits inset from the
    // window's real edges, the window itself has to be that much bigger so
    // `frame` still ends up the same visible size as before.
    width: 1320 + shadowMargin * 2
    height: 860 + shadowMargin * 2
    // Nothing below stops the window from being dragged narrower/shorter
    // than this on its own -- the custom resize grips near the end of this
    // file call the real window.startSystemResize(), so minimumWidth/
    // minimumHeight are enforced by the OS during that drag exactly like a
    // normal titled window's would be. Below ~860 (visible-content terms;
    // see the `+ shadowMargin * 2` note above) the fixed-width sidebar
    // (330px, home to the "Open port" button among other things) leaves the
    // icon toolbar/data monitor no room left to lay out in, and a language
    // with longer translations squeezes the Serial/Device commands tab
    // labels down to unreadable ellipsis well before that -- this is the
    // width below which the layout has nowhere left to give, not a number
    // tuned to look nice.
    minimumWidth: 860 + shadowMargin * 2
    minimumHeight: 600 + shadowMargin * 2
    visible: true
    title: qsTr("UbiBot Serial Assistant")
    // Transparent, not Theme.background -- this window now has real empty
    // (alpha) space around `frame` for the shadow below to occupy, and a
    // window can't selectively be "opaque here, see-through there": once
    // any part of it needs to show the desktop through, the whole surface
    // has to support per-pixel alpha. `frame` below draws its own opaque
    // Theme.background fill first thing, so nothing that used to assume an
    // opaque window backdrop (e.g. transparent-filled Rectangles layered on
    // top of it) changed as far as they can tell.
    color: "transparent"

    // Drops the OS-native title bar -- on Windows that's always a plain
    // white/system-colored strip no matter what theme the app itself is
    // in, so it stayed a jarring bright band above an otherwise fully dark
    // UI. TitleBar below replaces it with one styled off Theme like
    // everything else. The trade-off: Windows' own resize cursors/borders
    // and the DWM drop shadow disappear along with the native frame -- the
    // resize-grip MouseAreas inside `frame` below stand in for the resize
    // borders, `frame`'s own 1px outline stands in for the lost native
    // border (without ANY visible edge, this window was impossible to tell
    // apart from whatever sat directly behind it once that happened to be a
    // similar color), and the shadowMargin/MultiEffect setup below stands in
    // for the DWM shadow itself -- painted here in QML (a real, per-pixel
    // alpha-blurred shadow via MultiEffect, not an approximation) rather
    // than via a native per-platform call, since this app ships on
    // Windows/macOS/Linux and a from-QML shadow is the one approach that
    // doesn't need separate native code for each. This is also *why*
    // `header:`/ApplicationWindow's own automatic content layout isn't used
    // any more -- that mechanism always pins the header/content flush
    // against the window's own true edges with no way to inset it, which is
    // exactly what a shadow-margin window needs to not do; `frame` below
    // (a plain child, manually laid out) replaces it.
    flags: Qt.Window | Qt.FramelessWindowHint

    // Fusion (set in main.cpp) is one of the few built-in styles that
    // actually honors palette roles, so this is a cheap way to pull the
    // whole app's default control coloring toward the design's light/blue
    // theme without hand-skinning every control type. Popups/Dialogs don't
    // reliably inherit this from the ApplicationWindow, though -- each one
    // sets `palette: Theme.palette` itself too (see Theme.qml).
    palette: Theme.palette
    // Same story as `palette` above -- every plain Label/Control below that
    // doesn't set its own font.* inherits this, but historyPopup/
    // portErrorDialog further down (and every standalone Dialog in the
    // other qml files) get the same explicit assignment for the same
    // not-a-reliable-live-inherit reason.
    font.family: Theme.baseFontFamily
    font.pixelSize: Theme.baseFontSize

    Connections {
        target: AppController
        function onPortOpenFailed(error) { portErrorDialog.text = error; portErrorDialog.open() }
        function onStatusMessage(text) { statusToast.show(text) }
    }

    // Fusion's default ToolButton padding is generous (meant for
    // text+icon buttons); the design's toolbar is a tight row of plain
    // 32x32 icon squares, so the toolbar below uses this instead of relying
    // on the style default. Inline components must live at the document's
    // top level, hence declaring it here rather than inside `frame` below.
    component CompactToolButton: ToolButton {
        display: AbstractButton.IconOnly
        implicitWidth: 32
        implicitHeight: 32
        padding: 4
        // Every icons/*.svg is drawn with a hardcoded dark stroke
        // (Theme.text's light-mode value) and no `icon.color` of its own
        // to recolor it by -- fine against the (light) toolbar it was
        // designed for, invisible once the toolbar goes dark. Setting this
        // here (rather than on each of the icons below) recolors all of
        // them at once.
        icon.color: Theme.text
        ToolTip.visible: hovered
        ToolTip.delay: 400
    }

    // Replacement for the OS-native title bar removed by the frameless
    // `flags:` above -- same Theme.surface tone as everything else instead
    // of Windows' fixed white/system color, with hand-drawn (no new SVG
    // assets) minimize/maximize/close glyphs on the right.
    component TitleBar: Rectangle {
        id: titleBar
        implicitHeight: 32
        color: Theme.surface

        // The top-edge/top-corner resize grips used to live here rather
        // than alongside frame's other resize MouseAreas, back when this
        // was the one strip of the window that sat *above* `contentItem`
        // (what ApplicationWindow's automatic `header:` layout put a plain
        // child's own top edge below, not at the window's true top). Now
        // that TitleBar is just an ordinary first row inside `frame`'s own
        // manually-laid-out content (see the `flags:` comment up top for
        // why), there's no more coordinate-space split to work around --
        // the top edge/corners moved out to sit with the other five grips
        // (bottom/left/right + two bottom corners) instead, all uniformly
        // relative to `frame` now.

        // Drag-to-move: covers the whole bar. startSystemMove() on a plain
        // press (not a drag gesture of our own) is the standard QtQuick
        // pattern for this -- Qt only actually moves the window if the
        // mouse goes on to move while still held, so a press that turns
        // into a click (or the first half of the double-click below)
        // harmlessly no-ops.
        MouseArea {
            anchors.fill: parent
            onPressed: (mouse) => { if (mouse.button === Qt.LeftButton) window.startSystemMove() }
            onDoubleClicked: window.visibility = (window.visibility === Window.Maximized) ? Window.Windowed : Window.Maximized
        }

        RowLayout {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 12
            spacing: 10
            Image { source: "qrc:/icons/app.png"; sourceSize: Qt.size(20, 20); width: 16; height: 16 }
            Label { text: window.title; color: Theme.text; font.pixelSize: Theme.baseFontSize }
            // Version shown here (not appended to window.title itself) so
            // the OS-facing title -- taskbar button tooltip, alt-tab, etc.
            // -- stays just the plain app name; only this custom title
            // bar's own label needs it. AppController.appVersion ultimately
            // comes from CMakeLists.txt's `project(VERSION ...)` -- see the
            // comment there for how to bump it on release.
            Label {
                text: "v" + AppController.appVersion
                color: Theme.textMuted
                font.pixelSize: Theme.baseFontSize - 1
            }
        }

        Row {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            spacing: 0

            CaptionButton {
                onClicked: window.showMinimized()
                Rectangle { anchors.centerIn: parent; width: 10; height: 1; color: Theme.text }
            }
            CaptionButton {
                onClicked: window.visibility = (window.visibility === Window.Maximized) ? Window.Windowed : Window.Maximized
                Rectangle {
                    visible: window.visibility !== Window.Maximized
                    anchors.centerIn: parent
                    width: 10; height: 10
                    color: "transparent"
                    border.color: Theme.text
                    border.width: 1
                }
                // Restore glyph: two overlapping squares -- the front one
                // filled with the bar's own color so it reads as cutting
                // into the back one, same as the standard Windows icon.
                Item {
                    visible: window.visibility === Window.Maximized
                    anchors.centerIn: parent
                    width: 12; height: 12
                    Rectangle { x: 3; y: 0; width: 8; height: 8; color: "transparent"; border.color: Theme.text; border.width: 1 }
                    Rectangle { x: 0; y: 3; width: 8; height: 8; color: Theme.surface; border.color: Theme.text; border.width: 1 }
                }
            }
            CaptionButton {
                danger: true
                onClicked: window.close()
                Item {
                    anchors.centerIn: parent
                    width: 12; height: 12
                    Rectangle { anchors.centerIn: parent; width: 14; height: 1.4; color: Theme.text; rotation: 45 }
                    Rectangle { anchors.centerIn: parent; width: 14; height: 1.4; color: Theme.text; rotation: -45 }
                }
            }
        }
    }

    // One button shape shared by TitleBar's minimize/maximize/close --
    // `danger` switches on the standard red hover fill for Close. This
    // can't be nested inside the `component TitleBar` above (QML doesn't
    // allow nested inline components), so it's declared at the same
    // top level as TitleBar/CompactToolButton instead; its height just
    // matches TitleBar's own fixed 32 rather than referencing it directly
    // since a sibling inline component can't see into TitleBar's id scope.
    component CaptionButton: Rectangle {
        id: capBtn
        property bool danger: false
        signal clicked()
        implicitWidth: 46
        implicitHeight: 32
        color: hoverArea.containsMouse ? (danger ? "#e81123" : Theme.divider) : "transparent"
        MouseArea {
            id: hoverArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: capBtn.clicked()
        }
    }

    // `frame` is everything that used to be split across ApplicationWindow's
    // automatic `header:` slot and its plain `contentItem` children -- see
    // the `flags:` comment up top for why that split is gone now (in short:
    // `header:` always sits flush against the window's own true edges, with
    // no way to inset it for shadowMargin). Manually laid out here instead:
    // one ColumnLayout with the title bar/menu/toolbar block as its first
    // row and the existing body content (sidebar + data monitor) as its
    // second, wrapped in a plain Item sized `activeMargin` in from the
    // window's real edges on all four sides.
    Rectangle {
        id: frameBackdrop
        anchors.fill: frame
        // Itself invisible -- drawn only *through* the MultiEffect below
        // (same reasoning as DialogCard.qml's own `card`: MultiEffect
        // already renders an unmodified copy of its `source` plus the
        // shadow behind it, so drawing this a second time on top would be
        // redundant; staying invisible doesn't stop it from being a valid
        // effect source, see DialogCard.qml's comment for why).
        //
        // What it's actually for: frame's own children each paint their
        // own opaque backgrounds already (Theme.surface for the header
        // block, Theme.background/surface for the body panels below) --
        // this exists only as an opaque backstop behind the handful of
        // deliberately `color: "transparent"` Rectangles among them (the
        // sidebar container, the status-bar strip, ...), which used to see
        // `window.color`'s own opaque Theme.background showing through
        // directly. Now that window.color is "transparent" (see above),
        // nothing sits behind `frame` any more unless this does.
        visible: false
        color: Theme.background
    }

    Item {
        id: frame
        anchors.fill: parent
        anchors.margins: window.activeMargin

        ColumnLayout {
            id: headerColumn
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 0

            TitleBar { Layout.fillWidth: true }

            MenuBar {
                Layout.fillWidth: true
                Menu {
                    title: qsTr("&File")
                    MenuItem { text: qsTr("Save log"); onTriggered: saveLogDialog.open() }
                    MenuSeparator {}
                    MenuItem { text: qsTr("Exit"); onTriggered: Qt.quit() }
                }
                Menu {
                    title: qsTr("&Edit")
                    MenuItem { text: qsTr("Clear"); onTriggered: AppController.logModel.clear() }
                }
                Menu {
                    title: qsTr("&View")
                    MenuItem {
                        text: qsTr("Pause scrolling")
                        checkable: true
                        checked: monitor.paused
                        onTriggered: monitor.paused = checked
                    }
                }
                Menu {
                    title: qsTr("&Tools")
                    MenuItem { text: qsTr("Settings"); onTriggered: settingsDialog.open() }
                }
                Menu {
                    title: qsTr("&Help")
                    // Commented out (not removed) -- the self-update feature
                    // needs ubibot-appcenter's Software/Version backend to
                    // actually have this app registered (see
                    // docs/app-self-update.md#2) plus a real Version row
                    // uploaded for it, and neither has been deployed yet.
                    // Re-enable by uncommenting once that's live; nothing
                    // else needs to change -- AppController/
                    // SoftwareUpdateClient/SoftwareUpdateDialog.qml are all
                    // still fully wired up, this menu item was the only
                    // user-facing entry point into any of it.
                    // Named "software update" (not just "Check for
                    // updates") specifically to not collide with the
                    // unrelated "Check for updates" button already living
                    // in Settings & About's "Command library" section
                    // (device command data, not the app itself -- see
                    // SettingsAboutDialog.qml).
                    // MenuItem {
                    //     text: qsTr("Check for software update")
                    //     onTriggered: {
                    //         AppController.checkForAppUpdate();
                    //         softwareUpdateDialog.open();
                    //     }
                    // }
                    // MenuSeparator {}
                    // Was a copy-pasted duplicate of the Tools menu's own
                    // "Settings" item (same text, same handler) -- Settings
                    // already has its proper home under Tools, so this slot
                    // is About instead, per user feedback.
                    MenuItem { text: qsTr("About"); onTriggered: aboutDialog.open() }
                }
            }

            // Icon-only toolbar (matches the original design's compact 32x32
            // icon buttons) with the current-device badge and the
            // open/close-port button at the trailing end, all in the one row
            // the design puts them in.
            ToolBar {
                Layout.fillWidth: true
                // Design's toolbar row is a fixed 52px band (32px icon buttons
                // plus 10px of breathing room top and bottom) -- pin it
                // explicitly since Fusion's default ToolBar padding varies by
                // platform.
                implicitHeight: 52

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 12
                    // Was 0 (touching icon squares, by design -- see
                    // CompactToolButton's own comment) -- per user feedback that
                    // the toolbar buttons read as too cramped together, a small
                    // gap now separates them without losing the compact-toolbar
                    // look (still much tighter than the rest of the app's
                    // control spacing).
                    spacing: 4

                    CompactToolButton {
                        icon.source: "qrc:/icons/save.svg"
                        ToolTip.text: qsTr("Save log")
                        onClicked: saveLogDialog.open()
                    }
                    ToolSeparator { Layout.preferredHeight: 22; Layout.leftMargin: 4; Layout.rightMargin: 4 }
                    CompactToolButton {
                        icon.source: "qrc:/icons/send.svg"
                        ToolTip.text: qsTr("Send")
                        onClicked: AppController.sendManualText()
                    }
                    CompactToolButton {
                        icon.source: "qrc:/icons/pause.svg"
                        checkable: true
                        checked: monitor.paused
                        ToolTip.text: qsTr("Pause scrolling")
                        onToggled: monitor.paused = checked
                    }
                    CompactToolButton {
                        icon.source: "qrc:/icons/clear.svg"
                        ToolTip.text: qsTr("Clear")
                        onClicked: AppController.logModel.clear()
                    }
                    ToolSeparator { Layout.preferredHeight: 22; Layout.leftMargin: 4; Layout.rightMargin: 4 }
                    CompactToolButton {
                        icon.source: "qrc:/icons/settings.svg"
                        ToolTip.text: qsTr("Settings")
                        onClicked: settingsDialog.open()
                    }

                    Item { Layout.fillWidth: true }
                }
            }
        } // headerColumn

        // Body: sidebar + data monitor, filling everything below the
        // title/menu/toolbar block above. Used to be `anchors.fill: parent`
        // as its own separate plain child of the window (contentItem's only
        // real content, everything above having lived in the `header:`
        // slot instead) -- now that both live inside `frame` together, this
        // is anchored to headerColumn's own bottom instead of independently
        // filling its parent, so the two stack without overlapping.
        ColumnLayout {
            anchors.top: headerColumn.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            spacing: 0

        // --- body: left mode-specific panel + right data monitor -------------
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            Rectangle {
                Layout.preferredWidth: 330
                Layout.fillWidth: false
                Layout.fillHeight: true
                color: "transparent"
                border.color: Theme.divider
                border.width: 1
                ColumnLayout {
                    anchors.fill: parent
                    // The three-way mode switch lives inside the sidebar itself
                    // (full-width, equal thirds), not in the top toolbar --
                    // matching the original design's segmented control rather
                    // than competing for space with the toolbar's icons and
                    // device badge. Hand-rolled with Row + Repeater instead of
                    // TabBar/TabButton: TabBar's internal item positioner
                    // doesn't reliably honor per-tab width overrides, which is
                    // exactly what caused the earlier label-truncation bug.
                    // Design insets the segmented control 14px from the sidebar's
                    // edges inside a 61px-tall band, rather than butting it up
                    // against the sidebar's own edges.
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 61

                        Row {
                            id: modeBar
                            anchors.centerIn: parent
                            width: parent.width - 28
                            // Serial comes first (and is the tab shown on
                            // launch, via currentIndex's own default below)
                            // -- this is first and foremost a generic
                            // serial terminal, not a UbiBot-specific tool,
                            // so the port/baud/framing settings anyone with
                            // any serial device needs take priority over
                            // the UbiBot device command library.
                            property int currentIndex: 0
                            readonly property var labels: [qsTr("Serial"), qsTr("Device commands")]

                            Repeater {
                                model: modeBar.labels
                                delegate: Rectangle {
                                    id: segment
                                    required property string modelData
                                    required property int index
                                    width: Math.floor(modeBar.width / 2)
                                    height: 36
                                    // This is a fixed even split by design (a
                                    // two-way toggle reads as "equal choices"
                                    // -- an uneven one looks like a mistake),
                                    // so a language whose translation just
                                    // doesn't fit half this width can't be
                                    // fixed by resizing the segment itself.
                                    // clip + elide below is the fallback:
                                    // truncate with an ellipsis rather than
                                    // (Text doesn't clip on its own) bleeding
                                    // over the divider into the next segment,
                                    // which is what a long Russian label did.
                                    clip: true
                                    color: modeBar.currentIndex === index ? Theme.accent : "transparent"
                                    border.color: Theme.divider
                                    border.width: 1

                                    Label {
                                        anchors.centerIn: parent
                                        width: Math.min(implicitWidth, parent.width - 12)
                                        horizontalAlignment: Text.AlignHCenter
                                        elide: Text.ElideRight
                                        text: segment.modelData
                                        font.pixelSize: Theme.baseFontSize + 1
                                        color: modeBar.currentIndex === index ? Theme.accentForeground : Theme.text
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: modeBar.currentIndex = segment.index
                                    }
                                }
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.divider }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "transparent"
                        Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Theme.divider }

                        StackLayout {
                            anchors.fill: parent
                            currentIndex: modeBar.currentIndex

                            // Order matches modeBar.labels above: Serial,
                            // then Device commands, then Remote support.
                            SerialSettingsPanel { id: serialPanel }
                            CommandLibraryPanel {
                                onOpenParams: (row) => paramsPanel.openForRow(row)
                            }
                            RemoteAssistPanel { }
                        }
                    }
                }

            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                DataMonitorView {
                    id: monitor
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapLines: serialPanel.wrapLines
                    onSaveLogRequested: saveLogDialog.open()
                }

                CommandParamsPanel {
                    id: paramsPanel
                    Layout.fillWidth: true
                    Layout.margins: visible ? 16 : 0
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: 14
                    spacing: 12

                    ScrollView {
                        id: inputScroll
                        Layout.fillWidth: true
                        // Starts at the original fixed 74px. Multi-line
                        // content beyond that grows the box (so short input
                        // still looks the same as before) up to 200px, past
                        // which it stops growing and a scrollbar takes over
                        // instead -- otherwise overflowing lines were simply
                        // invisible with no way to reach them.
                        Layout.preferredHeight: Math.max(74, Math.min(200, inputField.contentHeight + 20))
                        clip: true

                        TextArea {
                            id: inputField
                            font.family: Theme.monoFont
                            placeholderText: qsTr("Type data to send…")
                            // Fusion's default placeholder color is a fixed
                            // gray tuned for a light background -- barely
                            // distinguishable from this box's own dark-mode
                            // background (Theme.surface). Theme.textMuted
                            // is already the "dim but legible" color for
                            // both themes.
                            placeholderTextColor: Theme.textMuted
                            // Deliberately not a `text: AppController.draftText`
                            // binding -- QML drops a property's binding as soon
                            // as anything assigns to it directly, and every
                            // keystroke does exactly that to `text`. After the
                            // user's first keystroke this binding would be gone
                            // for good, so external changes to draftText (Clear,
                            // double-clicking a history entry below) would stop
                            // reaching the box. The Connections handler below
                            // does that sync imperatively instead, which keeps
                            // working no matter how the user has edited the text.
                            Component.onCompleted: text = AppController.draftText
                            onTextChanged: if (text !== AppController.draftText) AppController.draftText = text
                            Connections {
                                target: AppController
                                function onDraftTextChanged() {
                                    if (inputField.text !== AppController.draftText) inputField.text = AppController.draftText
                                }
                            }

                            // Was missing an explicit `color:` -- a bare
                            // Rectangle defaults to white, which happened to
                            // look right against the old fixed light theme
                            // but stayed glaring white against a dark one.
                            background: Rectangle { color: Theme.surface; border.color: Theme.divider; border.width: 1 }
                        }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 116
                        Layout.fillWidth: false
                        Button { text: qsTr("Send"); highlighted: true; Layout.fillWidth: true; onClicked: AppController.sendManualText() }
                        Button { text: qsTr("Clear"); Layout.fillWidth: true; onClicked: AppController.clearDraft() }
                        Button {
                            id: historyButton
                            text: qsTr("History")
                            Layout.fillWidth: true
                            onClicked: historyPopup.visible ? historyPopup.close() : historyPopup.open()
                        }
                    }
                }

                // Recently-sent manual text, newest first. Opens upward from
                // the History button since this row sits near the bottom of
                // the window -- opening downward would mostly land off-screen.
                Popup {
                    id: historyPopup
                    parent: historyButton
                    x: historyButton.width - width
                    y: -height - 6
                    width: 320
                    padding: 0
                    modal: false
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                    palette: Theme.palette
                    font.family: Theme.baseFontFamily
                    font.pixelSize: Theme.baseFontSize

                    // See DialogCard.qml for why this needs its own
                    // elevated surface + border + shadow instead of a
                    // plain Theme.background fill.
                    background: DialogCard {}

                    contentItem: ColumnLayout {
                        spacing: 0

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.margins: 10
                            Label { text: qsTr("Send history"); font.bold: true; Layout.fillWidth: true }
                            ToolButton {
                                text: qsTr("Clear history")
                                flat: true
                                enabled: historyList.count > 0
                                onClicked: AppController.commandHistoryModel.clear()
                            }
                        }
                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.divider }

                        Label {
                            visible: historyList.count === 0
                            text: qsTr("No history yet")
                            color: Theme.textMuted
                            font.pixelSize: Theme.baseFontSize
                            Layout.margins: 16
                            Layout.alignment: Qt.AlignHCenter
                        }

                        ListView {
                            id: historyList
                            visible: count > 0
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.min(contentHeight, 260)
                            clip: true
                            model: AppController.commandHistoryModel
                            delegate: ItemDelegate {
                                id: historyDelegate
                                required property string commandText
                                required property string timeText
                                width: ListView.view.width
                                // ItemDelegate's own implicit height ignores
                                // whatever is stuffed into it below, so size off
                                // the row's implicit height instead.
                                implicitHeight: historyRow.implicitHeight + 14

                                // Double-click (not single-click) loads the
                                // entry -- a single click while just browsing
                                // history shouldn't clobber whatever the user
                                // is currently typing.
                                onDoubleClicked: {
                                    AppController.draftText = historyDelegate.commandText
                                    historyPopup.close()
                                }

                                RowLayout {
                                    id: historyRow
                                    anchors.fill: parent
                                    anchors.margins: 9
                                    Label {
                                        text: historyDelegate.commandText
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                        font.family: Theme.monoFont
                                        font.pixelSize: Theme.baseFontSize
                                    }
                                    Label {
                                        text: historyDelegate.timeText
                                        visible: historyDelegate.timeText.length > 0
                                        color: Theme.textMuted
                                        font.pixelSize: Theme.baseFontSize - 2
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: "transparent"
                    Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: Theme.divider }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 18

                        Label {
                            text: AppController.portStatusText
                            color: AppController.portOpen ? Theme.accent700 : Theme.error
                            font.family: Theme.monoFont
                            font.pixelSize: Theme.baseFontSize - 1
                        }
                        Label { text: AppController.portSummary; font.family: Theme.monoFont; font.pixelSize: Theme.baseFontSize - 1 }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: statusToast.visibleText
                            font.pixelSize: Theme.baseFontSize - 1
                            color: Theme.textMuted
                        }
                    }
                }
            }
        }
        } // closes the body ColumnLayout (re-anchored below headerColumn, see its own comment above)

        // --- frameless-window resize grips ----------------------------------
        // Stand in for the native resize borders lost along with the OS frame
        // (see the `flags:` comment up top). All eight (four edges, four
        // corners) are relative to `frame` now -- there's no more
        // header/contentItem coordinate-space split to work around (see
        // TitleBar's own comment, where the top-edge/top-corner three of
        // these used to live instead).
        MouseArea {
            visible: window.shadowActive
            height: window.resizeGrip
            anchors { top: parent.top; left: parent.left; right: parent.right }
            cursorShape: Qt.SizeVerCursor
            onPressed: window.startSystemResize(Qt.TopEdge)
        }
        MouseArea {
            visible: window.shadowActive
            height: window.resizeGrip
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            cursorShape: Qt.SizeVerCursor
            onPressed: window.startSystemResize(Qt.BottomEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            cursorShape: Qt.SizeHorCursor
            onPressed: window.startSystemResize(Qt.LeftEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
            cursorShape: Qt.SizeHorCursor
            onPressed: window.startSystemResize(Qt.RightEdge)
        }
        // Corners declared after the edges above so they win the overlapping
        // hit-test area (later siblings take hit-test priority).
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip * 2
            height: window.resizeGrip * 2
            anchors { top: parent.top; left: parent.left }
            cursorShape: Qt.SizeFDiagCursor
            onPressed: window.startSystemResize(Qt.TopEdge | Qt.LeftEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip * 2
            height: window.resizeGrip * 2
            anchors { top: parent.top; right: parent.right }
            cursorShape: Qt.SizeBDiagCursor
            onPressed: window.startSystemResize(Qt.TopEdge | Qt.RightEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip * 2
            height: window.resizeGrip * 2
            anchors { bottom: parent.bottom; left: parent.left }
            cursorShape: Qt.SizeBDiagCursor
            onPressed: window.startSystemResize(Qt.BottomEdge | Qt.LeftEdge)
        }
        MouseArea {
            visible: window.shadowActive
            width: window.resizeGrip * 2
            height: window.resizeGrip * 2
            anchors { bottom: parent.bottom; right: parent.right }
            cursorShape: Qt.SizeFDiagCursor
            onPressed: window.startSystemResize(Qt.BottomEdge | Qt.RightEdge)
        }

        // Whole-window perimeter border -- see the `flags:` comment up top:
        // going frameless dropped the OS's own window border/drop shadow
        // along with its title bar, and without some edge of its own this
        // window visually disappears into whatever's directly behind it
        // once that happens to be a similar color. One single Rectangle
        // now (used to be split into a header piece + three more at
        // window level, purely because `header:`/contentItem were
        // different coordinate spaces) -- declared last (topmost) so it
        // draws over the content, transparent fill + border-only so it
        // doesn't intercept clicks meant for anything underneath.
        Rectangle {
            anchors.fill: parent
            visible: window.shadowActive
            color: "transparent"
            border.color: Theme.dialogBorder
            border.width: 1
        }
    } // frame

    readonly property int resizeGrip: 4

    // The actual drop shadow -- MultiEffect draws frameBackdrop's alpha
    // silhouette (a plain opaque rectangle the same size/position as
    // `frame`, see its own comment above) blurred and offset behind it, a
    // real per-pixel blur rather than an approximation. Same technique
    // DialogCard.qml uses for every dialog/popup's own shadow; see that
    // file's comment for why MultiEffect over the older stacked-rectangle
    // trick. Hidden (not just zero-opacity -- no reason to keep it costing
    // anything) whenever `frame` itself has no margin to cast a shadow
    // into, i.e. while maximized.
    MultiEffect {
        anchors.fill: frameBackdrop
        source: frameBackdrop
        visible: window.shadowActive
        autoPaddingEnabled: true
        shadowEnabled: true
        shadowColor: "black"
        shadowOpacity: 0.35
        shadowBlur: 0.6
        shadowHorizontalOffset: 0
        shadowVerticalOffset: 3
        // frameBackdrop and frame are both declared *before* this in the
        // document, so without an explicit z this (declared later) would
        // win the default paint order and cover frame's own real content
        // instead of sitting behind it -- z: -1 overrides that.
        z: -1
    }

    // --- dialogs ---------------------------------------------------------
    SaveLogDialog { id: saveLogDialog }
    SettingsAboutDialog { id: settingsDialog }
    AboutDialog { id: aboutDialog }
    SoftwareUpdateDialog { id: softwareUpdateDialog }

    Dialog {
        id: portErrorDialog
        property alias text: errorLabel.text
        title: qsTr("Failed to open port")
        modal: true
        anchors.centerIn: Overlay.overlay
        palette: Theme.palette
        font.family: Theme.baseFontFamily
        font.pixelSize: Theme.baseFontSize
        // See DialogCard.qml for why this dialog needs its own elevated
        // surface + border + shadow instead of a plain Theme.background fill.
        background: DialogCard {}
        // See ModalDim.qml -- keeps this modal's dimming out of the
        // frameless main window's shadow-margin ring.
        Overlay.modal: ModalDim {}
        header: Label {
            text: portErrorDialog.title
            font.bold: true
            padding: 14
            color: Theme.text
            background: Rectangle { color: Theme.surface }
        }
        // Was `standardButtons: Dialog.Ok` -- see SettingsAboutDialog.qml's
        // root footer for why that auto-generated button had to be spelled
        // out explicitly instead (needs its own `palette:`).
        footer: DialogButtonBox {
            palette: Theme.palette
            background: Rectangle { color: Theme.surface }
            Button {
                text: qsTr("OK")
                palette: Theme.palette
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
        }
        Label { id: errorLabel; width: 320; wrapMode: Text.WordWrap; color: Theme.text }
    }

    // Transient status-bar message (e.g. "Log saved to ..."), matching the
    // old QStatusBar::showMessage(text, 5000) behavior.
    QtObject {
        id: statusToast
        property string visibleText: ""
        function show(text) {
            visibleText = text
            hideTimer.restart()
        }
        property Timer hideTimer: Timer {
            interval: 5000
            onTriggered: statusToast.visibleText = ""
        }
    }
}
