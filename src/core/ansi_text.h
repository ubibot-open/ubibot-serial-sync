#pragma once

#include <QString>

// Strips ANSI SGR color escape codes (the kind RT-Thread/msh and most
// embedded log frameworks emit -- "\x1b[32mok\x1b[0m") and other stray
// control bytes out of a raw device-output line, returning plain
// displayable text with none of that noise.
//
// This used to also *preserve* the color information, turning it into
// embedded HTML <span> runs so a device's own ANSI-colored output kept its
// own colors in the data monitor (and later, after that stopped scaling,
// a QSyntaxHighlighter that at least kept per-line TX/RX/SYS/ERR coloring
// without the HTML). The data monitor is plain, uncolored text now --
// simpler and lighter on memory for a data volume that can run into
// hundreds of thousands of lines, at the cost of losing that coloring
// entirely. A line with ANSI codes in it just has them stripped like any
// other control-byte noise; nothing renders them anymore.
namespace AnsiText {
QString stripToPlain(const QString &raw);
}
