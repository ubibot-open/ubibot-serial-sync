import QtQuick.Controls.Fusion as Fusion

// Shadows QtQuick.Controls' own TextArea app-wide -- see ComboBox.qml's
// comment for how/why. Fusion's own default padding is 6px (with an extra
// +4 on the left, which this inherits unchanged since that's a separate
// binding off of `padding`); bumped per user feedback that input boxes
// throughout the app read too short/cramped.
Fusion.TextArea {
    padding: 6
}
