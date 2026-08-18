import QtQuick
import QtQuick.Controls.Fusion as Fusion

// Shadows QtQuick.Controls' own CheckBox app-wide -- see ComboBox.qml's
// comment for how/why. Two complaints in one, per user feedback: the row
// itself (padding: 6 by default) felt cramped, and the checkbox square
// itself (a fixed 14x14 CheckIndicator, baked into Fusion's own CheckBox
// implementation) read as smaller than conventional desktop checkboxes.
//
// The indicator is created by the base type, not by this file, so its
// size can't be set with a plain property the way `padding` above can --
// a Binding targeting the already-instantiated indicator is the QML way
// to patch one property on a child object you didn't declare yourself,
// without having to reimplement CheckIndicator's own drawing (checkmark
// glyph, pressed/hover colors, focus outline, ...) from scratch here.
Fusion.CheckBox {
    id: control
    padding: 6

    Binding { target: control.indicator; property: "implicitWidth"; value: 18 }
    Binding { target: control.indicator; property: "implicitHeight"; value: 18 }
}
