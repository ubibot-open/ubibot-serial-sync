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
namespace AnsiText {
QString toRichText(const QString &raw, const QString &baseColor);
}
