import QtQuick
import QtQuick.Controls.Fusion as Fusion
import UbiBot

// Shadows QtQuick.Controls' own ComboBox for every file in this module --
// every .qml file here imports UbiBot last, after QtQuick.Controls, and QML
// resolves an unqualified type name exported by two different imports to
// whichever one was imported last. That silently routes every plain
// `ComboBox { ... }` in the app through this file instead, with no
// per-usage-site changes needed.
//
// Fusion's own ComboBox never sets its own `padding` (left at Control's
// default of 0), so its height comes almost entirely from its editable
// text field's own hardcoded 4px top/bottom padding -- noticeably shorter
// than VSCode's own dropdowns and, per user feedback, uncomfortably small
// to click.
Fusion.ComboBox {
    id: control
    leftPadding: 8

    // Deliberately NOT a `padding: N` / `topPadding: N` shorthand -- the
    // editable text field's built-in focus-ring rectangle (a child of that
    // field, sized to cover the *whole* outer control) only compensates for
    // a nonzero control.leftPadding in its own position math ("x: 1 -
    // control.leftPadding", cancelling the field's own left inset so the
    // ring still lands at the control's true left edge); it has no matching
    // compensation for topPadding. A nonzero topPadding shifts that field
    // (and the ring drawn inside it) down without one, so the ring's fixed
    // height ends up overflowing past the bottom of the now-shorter
    // available space -- the stray white border reported after picking a
    // value. Growing implicitHeight directly instead (Fusion's own formula,
    // just floored higher) gets a taller box without disturbing that.
    implicitHeight: Math.max(34, implicitBackgroundHeight + topInset + bottomInset,
                              implicitContentHeight + topPadding + bottomPadding,
                              implicitIndicatorHeight + topPadding + bottomPadding)

    // The dropdown list's own border -- Fusion.outline() is a private
    // helper that darkens whatever palette.window already is, which reads
    // fine against a light theme's near-white window but, against our dark
    // theme's already-near-black window, darkens straight into "practically
    // the same color as the popup's own fill" -- the border becomes all but
    // invisible exactly where a floating popup needs it most, to mark where
    // it ends and whatever's behind it begins. Theme.dialogBorder is
    // already tuned to stay visible in both themes (see DialogCard.qml, the
    // same problem for dialogs); Binding is the only way to patch one
    // property on this popup/background pair since Fusion's own ComboBox.qml
    // creates them itself, not this file.
    Binding { target: control.popup.background; property: "border.color"; value: Theme.dialogBorder }

    // The popup's ListView drives its own currentIndex from
    // control.highlightedIndex (Fusion's own binding, in its ComboBox.qml),
    // which starts at 0 on open and only moves via mouse-hover/keyboard from
    // there -- it does *not* follow control.currentIndex, so without this, a
    // combo box scrolled to (say) row 30 of 40 opens still scrolled to the
    // top, with row 0 misleadingly lit up blue instead of the actual current
    // value. An HTML <select> opens already scrolled to, and marking, its
    // current value -- this one-time positionViewAtIndex() on open (not a
    // binding -- that would permanently fight the ListView's own currentIndex
    // binding above instead of just nudging it once) gets the same result.
    Connections {
        target: control.popup
        function onOpened() {
            control.popup.contentItem.positionViewAtIndex(control.currentIndex, ListView.Center)
        }
    }

    // Fusion's own popup delegate (reproduced below, only the `background:`
    // is new) only ever bolds the text of the row matching currentIndex --
    // easy to miss at a glance in a longer list (baud rate, font family,
    // ...), and *not* the same row the blue hover highlight sits on unless
    // the mouse happens to already be over it. An HTML <select> marks its
    // currently-chosen option clearly the moment it opens, hover or not --
    // per user feedback, this should too.
    delegate: Fusion.MenuItem {
        id: menuItem
        required property var model
        required property int index
        readonly property bool isCurrent: control.currentIndex === index

        width: ListView.view.width
        text: model[control.textRole]
        font.weight: isCurrent ? Font.DemiBold : Font.Normal
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: 20
            // Fusion.Fusion, not Fusion.highlight() -- "Fusion" up at the top
            // of this file is a *namespace* alias for the whole
            // QtQuick.Controls.Fusion module (that's what makes
            // Fusion.MenuItem/Fusion.ComboBox below resolve), and the actual
            // helper singleton with .highlight()/.highlightedText() is a
            // type named "Fusion" *inside* that module -- one more level of
            // nesting than it looks like at first glance.
            color: menuItem.down || menuItem.highlighted
                   ? Fusion.Fusion.highlight(control.palette)
                   : (menuItem.isCurrent ? Theme.accentTint : "transparent")

            // Theme.accentTint alone (a light background wash) read too
            // close to the popup's own background color to notice at a
            // glance, per user feedback -- this solid accent-colored bar is
            // a stronger, unambiguous "this one" marker regardless of theme,
            // and (being a sibling on top of the fill above, not the fill
            // itself) still shows even when this same row is also hovered.
            //
            // Theme.accent800, not the plainer Theme.accent -- accentLight
            // ("#5980a6") is a muted, fairly desaturated blue-gray that
            // still didn't stand out enough against a light popup
            // background, per a follow-up report ("fine in dark mode, not
            // in light mode"). accent800 is the same "readable on top of
            // accentTint" variant the Settings dialog's own "Up to date"
            // badge already uses for exactly this reason -- a solid, darker
            // navy in light mode instead of a pale wash. accentDark's own
            // accent800 ("#cfe3f4", pale against near-black) reads just as
            // clearly, so this doesn't cost anything in dark mode either.
            Rectangle {
                visible: menuItem.isCurrent
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 3
                color: Theme.accent800
            }
        }
    }
}
