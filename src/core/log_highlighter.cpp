#include "core/log_highlighter.h"

#include <QTextCharFormat>

LogHighlighter::LogHighlighter(QObject *parent) : QSyntaxHighlighter(parent) {}

void LogHighlighter::setDocument(QQuickTextDocument *document) {
    if (quickDocument_ == document) return;
    quickDocument_ = document;
    // QSyntaxHighlighter::setDocument() (the base class's, taking a plain
    // QTextDocument*) is what actually wires this up -- QQuickTextDocument
    // is just the QML-facing handle TextEdit.textDocument hands out.
    QSyntaxHighlighter::setDocument(document ? document->textDocument() : nullptr);
}

void LogHighlighter::setTimestampColor(const QColor &color) {
    if (timestampColor_ == color) return;
    timestampColor_ = color;
    rehighlight();
}

void LogHighlighter::setTxColor(const QColor &color) {
    if (txColor_ == color) return;
    txColor_ = color;
    rehighlight();
}

void LogHighlighter::setRxColor(const QColor &color) {
    if (rxColor_ == color) return;
    rxColor_ = color;
    rehighlight();
}

void LogHighlighter::setSysColor(const QColor &color) {
    if (sysColor_ == color) return;
    sysColor_ = color;
    rehighlight();
}

void LogHighlighter::setErrColor(const QColor &color) {
    if (errColor_ == color) return;
    errColor_ = color;
    rehighlight();
}

QColor LogHighlighter::colorForTag(QStringView tag) const {
    if (tag == QLatin1String("TX ")) return txColor_;
    if (tag == QLatin1String("RX ")) return rxColor_;
    if (tag == QLatin1String("SYS")) return sysColor_;
    if (tag == QLatin1String("ERR")) return errColor_;
    return {};
}

void LogHighlighter::highlightBlock(const QString &text) {
    int pos = 0;

    // Optional "HH:mm:ss  " prefix -- LogListModel only ever emits this
    // exact shape (6 digits at fixed offsets, colons at 2/5, two spaces at
    // 8/9) when showTimestamp is on.
    if (text.size() >= 10 && text.at(2) == QLatin1Char(':') && text.at(5) == QLatin1Char(':') &&
        text.at(8) == QLatin1Char(' ') && text.at(9) == QLatin1Char(' ')) {
        bool isTimestamp = true;
        for (int i : {0, 1, 3, 4, 6, 7}) isTimestamp = isTimestamp && text.at(i).isDigit();
        if (isTimestamp) {
            QTextCharFormat format;
            format.setForeground(timestampColor_);
            setFormat(0, 8, format);
            pos = 10;
        }
    }

    // Dir tag: always exactly "TX "/"RX "/"SYS"/"ERR" (LogKind's own fixed,
    // padded-to-3 form) followed by two spaces -- see
    // LogListModel::linePlainText(). Anything else here means this isn't a
    // line shape this highlighter recognizes; leave it with no formatting
    // at all rather than guessing.
    if (text.size() < pos + 5) return;
    const QColor color = colorForTag(QStringView(text).mid(pos, 3));
    if (!color.isValid() || text.at(pos + 3) != QLatin1Char(' ') || text.at(pos + 4) != QLatin1Char(' ')) return;

    QTextCharFormat format;
    format.setForeground(color);
    setFormat(pos, 3, format);
    const int contentStart = pos + 5;
    if (contentStart < text.size()) setFormat(contentStart, text.size() - contentStart, format);
}
