#pragma once

#include <QString>

// Converts a raw device-output string that may contain ANSI SGR color
// escape codes (the kind RT-Thread/msh and most embedded log frameworks
// emit -- "\x1b[32mok\x1b[0m") into the small HTML subset QML's
// Text.RichText understands, so the data monitor can show the same colored
// log output a real terminal would instead of flattening every line to one
// solid color and leaving the raw escape bytes as visual noise.
//
// `baseColor` (a "#rrggbb" string) is used both as the run's starting color
// and as what bare resets (SGR 0/39) return to -- callers pass the
// direction color (TX/RX/SYS/ERR) here so a line with no escape codes at
// all still renders in that color, unchanged from before this existed.
//
// `dark` picks which of two built-in 16-color ANSI palettes a numbered SGR
// code (e.g. "\x1b[32m") maps to -- one tuned for a dark terminal
// background, one for a light one (see ansiColor() in the .cpp). This
// exists because the data monitor's own background now follows the app's
// light/dark theme (LogListModel::setDarkPalette) rather than always being
// dark, and the dark-tuned palette's muted colors are hard to read on white.
namespace AnsiText {

struct Result {
    QString html;
    // How many characters `html` renders as once a rich-text engine has
    // parsed it -- NOT html.length(), which also counts invisible markup
    // (span tags, &nbsp; source text, the dropped escape-code bytes...).
    // Callers that need to remove exactly this text from a document later
    // (LogListModel::lineDocLength) need this, not the markup length.
    int length = 0;
};

Result toRichText(const QString &raw, const QString &baseColor, bool dark);

}  // namespace AnsiText
