import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Right-hand "data monitor" pane: a scrolling, color-coded view of every
// TX/RX/SYS/ERR line, with a small header showing line count and byte
// counters. Pauses purely at the view level -- AppController.logModel keeps
// recording regardless, so unpausing shows everything that happened while
// paused.
Item {
    id: root

    property bool paused: false
    property bool wrapLines: true

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            color: "transparent"
            border.width: 0
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14

                Label {
                    text: qsTr("Data monitor")
                    font.pixelSize: 11
                    font.letterSpacing: 1
                    color: Theme.textMuted
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: qsTr("%1 lines").arg(AppController.logModel.lineCount)
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.textMuted
                }
                Label {
                    text: qsTr("Rx %1 B").arg(AppController.logModel.rxBytes)
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.textMuted
                }
                Label {
                    text: qsTr("Tx %1 B").arg(AppController.logModel.txBytes)
                    font.family: Theme.monoFont
                    font.pixelSize: 11
                    color: Theme.textMuted
                }
            }
        }

        // The log area gets its own (very slightly darker) background so it
        // reads as a distinct panel from the surrounding chrome, rather than
        // blending into the window background -- matching the original
        // design's tinted data-monitor pane.
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.surface

            ListView {
                id: listView
                anchors.fill: parent
                leftMargin: 14
                rightMargin: 14
                topMargin: 10
                bottomMargin: 10
                clip: true
                model: AppController.logModel
                spacing: 2
                boundsBehavior: Flickable.StopAtBounds

                // Auto-scroll to the newest line unless the user paused or
                // has manually scrolled away from the bottom.
                property bool stickToBottom: true
                onCountChanged: if (!root.paused && stickToBottom) positionViewAtEnd()
                onContentYChanged: {
                    stickToBottom = (contentY + height >= contentHeight - 4)
                }

                delegate: RowLayout {
                    width: listView.width - listView.leftMargin - listView.rightMargin
                    spacing: 10

                    required property string time
                    required property string dir
                    required property string text
                    required property string color

                    Label {
                        text: parent.time
                        color: Theme.textMuted
                        font.family: Theme.monoFont
                        font.pixelSize: 12
                        visible: text.length > 0
                    }
                    Label {
                        text: parent.dir
                        color: parent.color
                        font.family: Theme.monoFont
                        font.pixelSize: 12
                        Layout.preferredWidth: 30
                    }
                    Label {
                        text: parent.text
                        color: parent.color
                        font.family: Theme.monoFont
                        font.pixelSize: 12
                        wrapMode: root.wrapLines ? Text.Wrap : Text.NoWrap
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
