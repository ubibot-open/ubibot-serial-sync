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

    // background/surface/text/accent also exist as fixed Light/Dark literal
    // pairs below (backgroundLight/backgroundDark, ...) -- see the
    // `lightPalette`/`darkPalette` comment near the bottom for why.
    readonly property color backgroundLight: "#f2f2f3"
    readonly property color backgroundDark: "#1c1d20"
    readonly property color background: dark ? backgroundDark : backgroundLight

    readonly property color surfaceLight: "#e9e9ea"
    readonly property color surfaceDark: "#26282c"
    readonly property color surface: dark ? surfaceDark : surfaceLight

    readonly property color textLight: "#1d1f20"
    readonly property color textDark: "#e6e6e8"
    readonly property color text: dark ? textDark : textLight

    readonly property color textMuted: dark ? "#97999e" : "#7a7a7d"

    readonly property color accentLight: "#5980a6"
    readonly property color accentDark: "#6f9fcc"
    readonly property color accent: dark ? accentDark : accentLight
    readonly property color accent700: dark ? "#a2c8e8" : "#416180"
    readonly property color accent800: dark ? "#cfe3f4" : "#2c455d"
    readonly property color accentTint: dark ? "#24313d" : "#eef6ff"
    readonly property color divider: dark ? "#3c3e44" : "#c9c9ca"
    // A Dialog/Popup's own background used to just be Theme.background with
    // a `divider`-colored 1px border -- but that's the *same* color as the
    // main window sitting behind it, so the border (already subtle by
    // design, for use as a quiet in-page separator) was the only thing
    // marking the dialog's edge, and it all but disappeared in dark mode
    // (divider "#3c3e44" against background "#1c1d20" from a few feet back
    // reads as one solid color). DialogCard.qml uses `surface` for the
    // dialog's fill (a real tonal step up from `background`, not the same
    // color) and this stronger, higher-contrast border on top of it.
    readonly property color dialogBorder: dark ? "#55585f" : "#a6a6a9"
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
    // better dark, and colored device log output needs a dark backdrop to
    // show up against at all -- but that meant a jarring dark rectangle
    // sitting in the middle of an otherwise light app. In dark mode this
    // already matches the rest of the app, so those values are unchanged;
    // light mode now just reuses the normal light tokens instead of its
    // own fixed dark ones.
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
    //
    // This used to be a *single* Palette object whose roles were bound to
    // theme.background/.text/etc, on the assumption that mutating those
    // properties live would propagate to every control that had done
    // `palette: Theme.palette`. It doesn't: Fusion-styled controls
    // (ComboBox, RadioButton, Button, GroupBox, DialogButtonBox, ...)
    // convert whatever Palette object gets assigned into a plain QPalette
    // *snapshot* at the moment the assignment happens, and never look at
    // that object again -- they don't listen for its own windowChanged/
    // textChanged/etc signals. Meanwhile `palette: Theme.palette` in each
    // control is itself a QML binding that only re-evaluates when the
    // *`Theme.palette` reference* changes -- and with one shared object
    // whose sub-properties just mutate in place, that reference never
    // changes, so the binding fires exactly once (control creation) and
    // never again. Net effect: reopening a dialog after a theme switch
    // looks fine (fresh snapshot at creation time), but toggling the theme
    // while a dialog is already open leaves every Fusion control's chrome
    // frozen on the old colors while plain `color: Theme.background`-style
    // bindings elsewhere update correctly -- exactly the bug reported
    // against SettingsAboutDialog/ConnectionWizardDialog.
    //
    // Fix: keep two fully-static Palette instances instead of one mutable
    // shared one, and have `palette` switch which object it points to via
    // a ternary that reads `dark` directly. That read makes `dark` a real
    // dependency of the *outer* `palette` binding, so flipping it changes
    // `palette`'s identity -- which does re-fire every consumer's
    // `palette: Theme.palette` binding and forces a fresh QPalette
    // snapshot, live, with no per-dialog workaround needed.
    readonly property Palette lightPalette: Palette {
        window: theme.backgroundLight
        windowText: theme.textLight
        base: theme.surfaceLight
        text: theme.textLight
        button: theme.surfaceLight
        buttonText: theme.textLight
        highlight: theme.accentLight
        highlightedText: theme.accentForeground
    }
    readonly property Palette darkPalette: Palette {
        window: theme.backgroundDark
        windowText: theme.textDark
        base: theme.surfaceDark
        text: theme.textDark
        button: theme.surfaceDark
        buttonText: theme.textDark
        highlight: theme.accentDark
        highlightedText: theme.accentForeground
    }
    readonly property Palette palette: dark ? darkPalette : lightPalette
}
