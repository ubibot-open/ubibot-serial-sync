#pragma once

#include <QString>

// Strips ANSI SGR color escape codes (the kind RT-Thread/msh and most
// embedded log frameworks emit -- "\x1b[32mok\x1b[0m") and other stray
// control bytes out of a raw device-output line, returning plain
// displayable text with none of that noise.
//
// This used to also *preserve* the color information, turning it into
// embedded HTML <span> runs so a device's own ANSI-colored output kept its
// own colors in the data monitor. That went away when the data monitor
// itself switched from one continuous rich-text document to a
// syntax-highlighted plain-text one (see LogHighlighter) -- rendering
// hundreds of thousands of lines of embedded per-line HTML doesn't scale;
// each line's <span> runs made every insert/remove on the document
// meaningfully more expensive, and a device dumping a large stored log
// made that add up to a UI freeze/stutter no amount of batching the
// inserts/removes themselves fully fixed. A line with ANSI codes in it now
// just renders in its TX/RX/SYS/ERR kind color like any other line, rather
// than whatever custom colors the device itself requested mid-line.
namespace AnsiText {
QString stripToPlain(const QString &raw);
}
