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

    // Data monitor only: a real terminal reads better dark, and colored
    // device log output (see ansi_text.h) needs a dark backdrop to show up
    // against at all -- these deliberately don't match the light palette
    // above. colorForKind() in log_list_model.cpp mirrors dividerMuted/
    // sent/sys/err so the TX/RX/SYS/ERR tag matches the line's base color.
    readonly property color consoleBackground: "#1b1f27"
    readonly property color consoleText: "#d8dce0"
    readonly property color consoleMuted: "#6b7280"
    readonly property color consoleBorder: "#33383f"

    readonly property string monoFont: "Consolas"

    readonly property int spacingSmall: 6
    readonly property int spacingMedium: 10
    readonly property int spacingLarge: 16
}
