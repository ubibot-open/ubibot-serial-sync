import QtQuick
import QtQuick.Controls.Fusion as Fusion
import UbiBot

// Shadows QtQuick.Controls' own Menu app-wide (the top MenuBar's own
// File/Edit/View/Tools/Help dropdowns, and CommandLibraryPanel.qml's
// right-click "My templates" edit/delete menu) -- see ComboBox.qml's
// comment for how/why the shadowing itself works, and its own Binding
// comment for why the border needs patching in dark mode: Fusion.outline()
// darkens palette.window, which in a light theme gives a visible gray but
// in our dark theme darkens what's already near-black into practically the
// same color as the menu's own fill -- the exact bug reported for
// ComboBox's own dropdown, same fix here.
Fusion.Menu {
    id: control
    Binding { target: control.background; property: "border.color"; value: Theme.dialogBorder }
}
