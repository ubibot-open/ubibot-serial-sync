import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// Rich port picker shared by SerialSettingsPanel and CommandLibraryPanel --
// both read/write the same AppController.selectedPortName, so whichever
// panel the user picks a port in, the other agrees on it (it's also what
// the toolbar's "Open port" button actually opens).
//
// Plain ComboBox + textRole: "displayLabel" used to render every row as one
// flat "COM4 (USB Serial)" line -- enough to tell ports apart, but not laid
// out for skimming several at a glance, and Windows' own port description is
// often a generic "USB Serial Device" that never actually says which bridge
// chip it is. This gives each row its own two-line delegate (port name +
// description) plus the recognized chip name ("CH340", "CP210x", ...) for
// whichever port matches one of UbiBot's known USB-serial chips (see
// SerialManager::availablePorts) -- closer to what a dedicated serial
// terminal's port picker usually looks like, and more useful than a bare
// unexplained "Recommended" tag (an earlier version of this file had one;
// it told you *that* a port looked right without saying *why*). The closed
// control itself stays a plain "COM4", only the open dropdown gets the
// richer per-row detail.
ComboBox {
    id: control

    model: AppController.portListModel
    displayText: currentIndex >= 0 ? AppController.portListModel.portNameAt(currentIndex)
                                    : qsTr("No ports found")
    // Popup defaults to exactly `control.width`, which is sized for the
    // compact closed label above, not for a port name plus its description
    // -- widen it enough for the descriptions the reference design shows
    // (e.g. "USB-SERIAL CH340") without truncating on every row.
    popup.width: Math.max(control.width, 280)
    // Fusion's default popup background is close enough to Theme.background
    // behind it (both are near-black in dark mode) that the dropdown had no
    // visible edge of its own -- DialogCard is the same fix already used for
    // Dialogs/Popups elsewhere (Theme.surface fill + a real border + a soft
    // shadow), reused here for the same reason.
    popup.background: DialogCard {}

    delegate: ItemDelegate {
        id: portDelegate
        required property int index
        required property string portName
        required property string description
        required property bool recommended
        required property string chipLabel
        width: control.popup.width
        highlighted: control.highlightedIndex === index
        // ItemDelegate's own implicit height ignores whatever's stuffed
        // into it below (same reasoning as every other list delegate in
        // this app -- see e.g. ConnectionWizardDialog's port/model lists).
        implicitHeight: portRow.implicitHeight + 16

        // Fusion draws the highlighted/hovered row's background in
        // Theme.accent -- Theme.text/.textMuted/.accent (this row's normal,
        // unhighlighted colors) are all tuned for contrast against
        // Theme.background/.surface, not against that mid-tone accent blue,
        // so hovering a row used to make its own text nearly disappear into
        // the highlight behind it. Theme.accentForeground is the color
        // already reserved for exactly this (see Theme.qml) -- text/icons
        // sitting on an accent-colored surface, same as a selected tab label.
        readonly property color rowText: highlighted ? Theme.accentForeground : Theme.text
        readonly property color rowMuted: highlighted ? Theme.accentForeground : Theme.textMuted
        readonly property color rowAccent: highlighted ? Theme.accentForeground : Theme.accent

        RowLayout {
            id: portRow
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            // Recognized-chip ports get a slim accent bar instead of
            // repeating the chip name on every single row's left edge --
            // the label on the right already spells that out.
            Rectangle {
                Layout.preferredWidth: 3
                Layout.fillHeight: true
                Layout.topMargin: 2
                Layout.bottomMargin: 2
                radius: 1.5
                color: portDelegate.recommended ? portDelegate.rowAccent : "transparent"
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1
                Label {
                    text: portDelegate.portName
                    font.bold: true
                    color: portDelegate.rowText
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Label {
                    // Ports Qt can't identify (a generic PTY, a virtual
                    // port with no USB descriptor, ...) report an empty
                    // description -- falls back to something better than
                    // a blank second line.
                    text: portDelegate.description.length > 0 ? portDelegate.description : qsTr("Unknown device")
                    font.pixelSize: 11
                    color: portDelegate.rowMuted
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            // Was a plain "Recommended" tag -- true, but didn't say why, so
            // it read as an unexplained value judgment. The chip name is
            // the actual reason (this is one of UbiBot's own USB-serial
            // bridge chips) and is useful on its own even without that
            // framing.
            Label {
                visible: portDelegate.chipLabel.length > 0
                text: portDelegate.chipLabel
                font.bold: true
                color: portDelegate.rowAccent
                font.pixelSize: 11
            }
        }
    }
}
