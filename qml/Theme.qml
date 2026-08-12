pragma Singleton
import QtQuick
import QtQuick.Controls
import UbiBot

// Shared design tokens, lifted from the original design mockup's light
// palette (see styles.h.bak in git history for the old Widgets version).
// Every panel imports this singleton rather than hardcoding colors.
//
// Every color below is a *binding* on `dark`, not a literal -- switching
// AppController.themeMode (Settings & About -> Theme) re-evaluates all of
// them, and since every panel already reads colors through this singleton
// rather than hardcoding values, the whole app re-colors live with no
// per-panel work needed for anything that reads Theme.* directly.
//
// Fusion-styled controls that *don't* set an explicit color (a plain
// `Label { text: "x" }`, say) instead default to reading `palette.windowText`
// et al -- see the `palette` property at the bottom, which every top-level
// Window/Dialog/Popup needs to set explicitly (`palette: Theme.palette`).
// Popups do NOT reliably inherit ApplicationWindow's own palette, so
// anything declared as its own Dialog/Popup (SettingsAboutDialog,
// SaveLogDialog, ConnectionWizardDialog, the history Popup, every
// standalone confirmation Dialog, ...) needs the explicit assignment or its
// plain Labels/Buttons stay stuck on Qt's system-default light colors even
// while the rest of the app is dark.
QtObject {
    id: theme

    readonly property bool dark: AppController.themeMode === "dark"

    readonly property color background: dark ? "#1c1d20" : "#f2f2f3"
    readonly property color surface: dark ? "#26282c" : "#e9e9ea"
    readonly property color text: dark ? "#e6e6e8" : "#1d1f20"
    readonly property color textMuted: dark ? "#97999e" : "#7a7a7d"
    readonly property color accent: dark ? "#6f9fcc" : "#5980a6"
    readonly property color accent700: dark ? "#a2c8e8" : "#416180"
    readonly property color accent800: dark ? "#cfe3f4" : "#2c455d"
    readonly property color accentTint: dark ? "#24313d" : "#eef6ff"
    readonly property color divider: dark ? "#3c3e44" : "#c9c9ca"
    readonly property color error: dark ? "#e58a87" : "#aa3333"
    // The readable color for text/icons drawn on top of an accent-colored
    // surface (selected tab/chip label, Fusion's palette.highlightedText,
    // selected-text foreground in the data monitor). Deliberately fixed
    // rather than reusing `background` the way the light palette's own
    // near-white background happened to double as this color -- `accent`
    // stays roughly the same mid-tone blue in both themes, but `background`
    // flips from near-white to near-black, which would make dark-mode text
    // unreadable against it.
    // NOTE: cannot be named "onAccent" -- QML reads any `on<Capital>`
    // property as a signal-handler assignment ("onAccent" -> handler for a
    // signal named "accent"), which fails to compile with "Cannot assign a
    // value to a signal (expecting a script to be run)".
    readonly property color accentForeground: "#f5f7fa"

    // Data monitor (right-hand pane) + its right-click context menu. Used
    // to be permanently dark regardless of theme -- a real terminal reads
    // better dark, and colored device log output (see ansi_text.h) needs a
    // dark backdrop to show up against at all -- but that meant a jarring
    // dark rectangle sitting in the middle of an otherwise light app.
    // In dark mode this already matches the rest of the app, so those
    // values are unchanged; light mode now just reuses the normal light
    // tokens instead of its own fixed dark ones. colorForKind() and
    // ansiColor() in log_list_model.cpp/ansi_text.cpp mirror these (and
    // consoleMuted below) by hand for the same two cases, since that's
    // plain C++ with no access to this singleton -- LogListModel::
    // setDarkPalette(), pushed by AppController, is what keeps it in sync.
    readonly property color consoleBackground: dark ? "#1b1f27" : background
    readonly property color consoleText: dark ? "#d8dce0" : text
    readonly property color consoleMuted: dark ? "#6b7280" : textMuted
    readonly property color consoleBorder: dark ? "#33383f" : divider

    readonly property string monoFont: "Consolas"

    readonly property int spacingSmall: 6
    readonly property int spacingMedium: 10
    readonly property int spacingLarge: 16

    // See the file-level comment above -- assign this to every top-level
    // Window/Dialog/Popup's own `palette` property (not just the main
    // ApplicationWindow) so Fusion's default control colors track the
    // current theme everywhere, not only where Theme.* is read directly.
    readonly property Palette palette: Palette {
        window: theme.background
        windowText: theme.text
        base: theme.surface
        text: theme.text
        button: theme.surface
        buttonText: theme.text
        highlight: theme.accent
        highlightedText: theme.accentForeground
    }
}
