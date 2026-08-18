import QtQuick.Controls.Fusion as Fusion

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
}
