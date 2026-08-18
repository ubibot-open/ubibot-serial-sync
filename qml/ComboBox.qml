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
// to click. Padding feeds directly into Control's own (untouched here)
// implicitHeight formula, so bumping it is enough to grow the box without
// reimplementing any of Fusion's own drawing.
Fusion.ComboBox {
    leftPadding: 8
    padding: 4
}
