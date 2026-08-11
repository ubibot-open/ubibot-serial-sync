pragma Singleton
import QtQuick

// Shared design tokens, lifted from the original design mockup's light
// palette (see styles.h.bak in git history for the old Widgets version).
// Every panel imports this singleton rather than hardcoding colors.
QtObject {
    readonly property color background: "#f2f2f3"
    readonly property color surface: "#e9e9ea"
    readonly property color text: "#1d1f20"
    readonly property color textMuted: "#7a7a7d"
    readonly property color accent: "#5980a6"
    readonly property color accent700: "#416180"
    readonly property color accent800: "#2c455d"
    readonly property color accentTint: "#eef6ff"
    readonly property color divider: "#c9c9ca"
    readonly property color error: "#aa3333"

    readonly property string monoFont: "Consolas"

    readonly property int spacingSmall: 6
    readonly property int spacingMedium: 10
    readonly property int spacingLarge: 16
}
