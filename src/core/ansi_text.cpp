#include "core/ansi_text.h"

#include <QStringList>

namespace {

// Standard 16-color ANSI palette, tuned for a dark terminal background
// rather than the washed-out primaries a literal xterm palette would give
// (pure "#008000" green disappears next to near-black; this is closer to
// the muted "One Dark"-style palette most modern terminal themes use).
QString ansiColor(int code) {
    switch (code) {
    case 30: return QStringLiteral("#5c6370");  // black -> dim gray, still visible
    case 31: return QStringLiteral("#e06c75");  // red
    case 32: return QStringLiteral("#98c379");  // green
    case 33: return QStringLiteral("#e5c07b");  // yellow
    case 34: return QStringLiteral("#61afef");  // blue
    case 35: return QStringLiteral("#c678dd");  // magenta
    case 36: return QStringLiteral("#56b6c2");  // cyan
    case 37: return QStringLiteral("#d8dce0");  // white
    case 90: return QStringLiteral("#7f848e");  // bright black
    case 91: return QStringLiteral("#f07178");  // bright red
    case 92: return QStringLiteral("#a6e3a1");  // bright green
    case 93: return QStringLiteral("#f9e2af");  // bright yellow
    case 94: return QStringLiteral("#82aaff");  // bright blue
    case 95: return QStringLiteral("#d3869b");  // bright magenta
    case 96: return QStringLiteral("#89ddff");  // bright cyan
    case 97: return QStringLiteral("#ffffff");  // bright white
    default: return QString();
    }
}

// Appends one plain (non-escape-sequence) character to `out`, HTML-escaped,
// and returns how many characters it renders as (0 for a dropped control
// byte, 4 for a tab, 1 otherwise) -- the caller accumulates this into
// Result::length, which is NOT html.length() (that also counts markup:
// span tags, "&nbsp;"/"&amp;" source text, the escape-code bytes this
// drops entirely...).
//
// Control bytes other than \n/\r/\t are dropped rather than passed through:
// a noisy line or a mid-frame baud mismatch can drop stray NUL/BEL/etc.
// bytes into the stream, and Qt's rich-text renderer draws those as visible
// tofu glyphs instead of silently ignoring them the way a real terminal would.
int appendEscaped(QString &out, QChar c) {
    switch (c.unicode()) {
    case '&': out += QStringLiteral("&amp;"); return 1;
    case '<': out += QStringLiteral("&lt;"); return 1;
    case '>': out += QStringLiteral("&gt;"); return 1;
    case '\n': out += QStringLiteral("<br/>"); return 1;
    case '\r': return 0;  // paired \r\n handled by the \n above; bare \r dropped
    case '\t': out += QStringLiteral("    "); return 4;
    default:
        if (c.unicode() >= 0x20) {
            out += c;
            return 1;
        }
        return 0;  // other C0 control byte -- drop.
    }
}

}  // namespace

AnsiText::Result AnsiText::toRichText(const QString &raw, const QString &baseColor) {
    QString html;
    html.reserve(raw.size() + 32);
    int length = 0;

    QString color = baseColor;
    bool bold = false;
    bool spanOpen = false;

    auto closeSpan = [&] {
        if (spanOpen) {
            html += QStringLiteral("</span>");
            spanOpen = false;
        }
    };
    auto openSpan = [&] {
        html += QStringLiteral("<span style=\"color:%1;%2\">")
                    .arg(color, bold ? QStringLiteral("font-weight:600;") : QString());
        spanOpen = true;
    };
    // Every text run (including the very first) needs its own span since
    // style can't be changed on an already-open one -- close then reopen
    // whenever color/bold actually changed, rather than on every escape
    // code (SGR sequences often arrive back-to-back with no text between).
    auto restartSpan = [&] {
        closeSpan();
        openSpan();
    };
    restartSpan();

    int i = 0;
    const int n = raw.size();
    while (i < n) {
        const QChar ch = raw.at(i);
        if (ch.unicode() == 0x1b && !(i + 1 < n && raw.at(i + 1) == QLatin1Char('['))) {
            // Lone ESC (or an escape type this parser doesn't recognize as
            // CSI, e.g. ESC alone at the end of a chunk) -- drop it rather
            // than let it render as a control-character glyph.
            ++i;
            continue;
        }
        if (ch.unicode() == 0x1b) {
            // CSI sequence: ESC [ <params> <final-byte>. Only the SGR form
            // (final byte 'm') carries color/style; everything else
            // (cursor movement, clear-line, ...) is consumed and dropped
            // silently so it can't leak into the visible text as junk.
            int j = i + 2;
            while (j < n && !raw.at(j).isLetter()) ++j;
            if (j >= n) break;  // unterminated escape at end of buffer -- stop, don't emit it

            const QChar final = raw.at(j);
            if (final == QLatin1Char('m')) {
                const QString params = raw.mid(i + 2, j - (i + 2));
                // A bare "\x1b[m" (empty params) is shorthand for "\x1b[0m"
                // (reset) -- split()ing "" with SkipEmptyParts would drop
                // it entirely and leave color/bold untouched, so give it an
                // explicit "0" instead of relying on the split.
                QStringList codes = params.split(QLatin1Char(';'), Qt::SkipEmptyParts);
                if (codes.isEmpty()) codes << QStringLiteral("0");
                bool colorChanged = false;
                for (const QString &part : codes) {
                    const int code = part.toInt();
                    if (code == 0) {
                        if (color != baseColor || bold) colorChanged = true;
                        color = baseColor;
                        bold = false;
                    } else if (code == 1) {
                        if (!bold) colorChanged = true;
                        bold = true;
                    } else if (code == 22) {
                        if (bold) colorChanged = true;
                        bold = false;
                    } else if (code == 39 || code == 49) {
                        if (color != baseColor) colorChanged = true;
                        color = baseColor;
                    } else {
                        const QString mapped = ansiColor(code);
                        if (!mapped.isEmpty()) {
                            if (color != mapped) colorChanged = true;
                            color = mapped;
                        }
                    }
                }
                if (colorChanged) restartSpan();
            }
            i = j + 1;
            continue;
        }
        length += appendEscaped(html, ch);
        ++i;
    }
    closeSpan();
    return {html, length};
}
