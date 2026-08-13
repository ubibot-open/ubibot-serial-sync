#include "core/ansi_text.h"

namespace AnsiText {

QString stripToPlain(const QString &raw) {
    QString out;
    out.reserve(raw.size());

    int i = 0;
    const int n = raw.size();
    while (i < n) {
        const QChar ch = raw.at(i);

        if (ch.unicode() == 0x1b) {
            if (i + 1 < n && raw.at(i + 1) == QLatin1Char('[')) {
                // CSI sequence: ESC [ <params> <final-byte> -- consume and
                // drop the whole thing regardless of what it is (SGR color,
                // cursor movement, clear-line, ...); none of it means
                // anything once rendered as plain text.
                int j = i + 2;
                while (j < n && !raw.at(j).isLetter()) ++j;
                if (j >= n) break;  // unterminated at the end of the buffer -- stop, don't emit it
                i = j + 1;
            } else {
                ++i;  // lone/unrecognized ESC -- drop just the one byte
            }
            continue;
        }

        switch (ch.unicode()) {
        case '\r':
            ++i;
            continue;  // bare \r dropped -- a paired \r\n is already handled by the caller
        case '\t':
            out += QStringLiteral("    ");
            ++i;
            continue;
        default:
            // Other C0 control bytes are dropped rather than passed
            // through -- a noisy line or a mid-frame baud mismatch can
            // drop stray NUL/BEL/etc. bytes into the stream, and Qt's text
            // renderer draws those as visible tofu glyphs instead of
            // silently ignoring them the way a real terminal would.
            if (ch.unicode() >= 0x20) out += ch;
            ++i;
        }
    }
    return out;
}

}  // namespace AnsiText
