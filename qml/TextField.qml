import QtQuick.Controls.Fusion as Fusion

// Shadows QtQuick.Controls' own TextField app-wide -- see ComboBox.qml's
// comment for how/why. Fusion's own default padding (4px each side) reads
// short next to VSCode's own input boxes, per user feedback.
Fusion.TextField {
    padding: 6
}
