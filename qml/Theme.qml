// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

pragma Singleton
import QtQuick
import QtQuick.Controls
import UbiBot

QtObject {
    id: theme

    readonly property bool dark: AppController.themeMode === "dark"
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
    readonly property color accentTint: dark ? "#24313d" : "#d7e7f5"
    readonly property color divider: dark ? "#3c3e44" : "#c9c9ca"
    readonly property color dialogBorder: dark ? "#55585f" : "#a6a6a9"
    readonly property color error: dark ? "#e58a87" : "#aa3333"
    readonly property color accentForeground: "#f5f7fa"
    readonly property color consoleBackground: dark ? "#1b1f27" : background
    readonly property color consoleText: dark ? "#d8dce0" : text
    readonly property color consoleMuted: dark ? "#6b7280" : textMuted
    readonly property color consoleBorder: dark ? "#33383f" : divider
    readonly property string monoFont: "Consolas"
    readonly property string baseFontFamily: AppController.systemFontFamily
    readonly property int baseFontSize: AppController.systemFontSize
    readonly property int spacingSmall: 8
    readonly property int spacingMedium: 12
    readonly property int spacingLarge: 18
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
