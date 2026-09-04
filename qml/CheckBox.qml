import QtQuick
import QtQuick.Controls.Fusion as Fusion
import QtQuick.Shapes

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

    // Same color Fusion's own (now-hidden) checkmark used -- see
    // CheckIndicator.qml's own `checkMarkColor`/210-alpha -- so swapping
    // the glyph doesn't also change its color.
    readonly property color checkMarkColor: Qt.darker(control.palette.text, 1.2)

    // Fusion's own checkmark glyph is a small raster PNG (checkmark.png,
    // drawn via a ColorImage with no width/height of its own -- see
    // CheckIndicator.qml) sized for the original 14x14 indicator. Scaling
    // *that* image up to fill the enlarged 18x18 box (tried first, at
    // various fractions up to the box's full size) just upscales those
    // same few source pixels -- more of them get stretched over more
    // screen pixels, which reads as soft/blurry rather than crisp, worse
    // the closer the scale gets to 1:1. Hiding it and drawing our own
    // checkmark as a vector Shape (below) instead sidesteps that entirely:
    // a ShapePath's fill is real geometry the scene graph tessellates at
    // whatever the actual screen resolution is (same reason the
    // minimize/maximize/close glyphs in Main.qml's CaptionButton are
    // drawn with plain Rectangles instead of images) -- there's no
    // fixed-resolution source pixels to run out of, so it stays sharp at
    // any indicator size or display scale factor.
    //
    // indicator.children[1] is that ColorImage specifically --
    // CheckIndicator.qml declares exactly three children in this fixed
    // order (a 1px top-shadow line, this checkmark image, then the
    // partially-checked square), so index 1 is reliable as long as that
    // file's own child order doesn't change; if a future Qt bump ever
    // makes the checkmark visible again, that's the first thing to check.
    Binding { target: control.indicator.children[1]; property: "visible"; value: false }

    Item {
        id: checkGlyph
        anchors.fill: control.indicator
        z: 1
        visible: control.checkState === Qt.Checked
                 || (control.checked && control.checkState === undefined)
        Shape {
            width: 50
            height: 50
            anchors.centerIn: parent
            scale: checkGlyph.width * 0.78 / width
            antialiasing: true

            ShapePath {
                fillColor: Qt.rgba(control.checkMarkColor.r, control.checkMarkColor.g,
                                    control.checkMarkColor.b, 210 / 255)
                strokeColor: fillColor
                strokeWidth: 3.8
                PathSvg {
                    path: "M 42.875 8.625 C 42.84375 8.632813 42.8125 8.644531 42.78125 8.65625 C 42.519531 8.722656 42.292969 8.890625 42.15625 9.125 L 21.71875 40.8125 L 7.65625 28.125 C 7.410156 27.8125 7 27.675781 6.613281 27.777344 C 6.226563 27.878906 5.941406 28.203125 5.882813 28.597656 C 5.824219 28.992188 6.003906 29.382813 6.34375 29.59375 L 21.25 43.09375 C 21.46875 43.285156 21.761719 43.371094 22.050781 43.328125 C 22.339844 43.285156 22.59375 43.121094 22.75 42.875 L 43.84375 10.1875 C 44.074219 9.859375 44.085938 9.425781 43.875 9.085938 C 43.664063 8.746094 43.269531 8.566406 42.875 8.625 Z"
                }
            }
        }
    }
}
