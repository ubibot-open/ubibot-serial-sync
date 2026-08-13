#pragma once

#include <QColor>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QSyntaxHighlighter>

// Attaches to the data monitor's TextEdit.textDocument (see DataMonitorView.
// qml) and colors each already-inserted plain-text line by recognizing its
// shape, rather than the view pre-building colored HTML the way this app
// used to (see ansi_text.h's comment for why that stopped scaling). Every
// line LogListModel ever writes to that document looks like one of:
//
//   "HH:mm:ss  TX  <content>"   (with a timestamp -- showTimestamp on)
//   "TX  <content>"             (without one -- showTimestamp off)
//
// with "TX" standing in for whichever of LogKind's own fixed, padded-to-3
// tags (TX/RX/SYS/ERR) applies. LogListModel is the *only* thing that ever
// writes to this document, so that shape is always exactly this -- no
// side-channel metadata (e.g. QTextBlockUserData set at insert time) is
// needed to tell highlightBlock() what a line is; it just pattern-matches
// the text QSyntaxHighlighter hands it directly.
//
// Colors are plain QML-settable properties bound straight to Theme.qml's
// tokens (see DataMonitorView.qml) rather than hardcoded/mirrored in C++ the
// way LogListModel's old colorForKind() had to -- change any of them and
// this re-highlights the whole document once to match, the same as a
// light/dark theme switch already made the rest of the app do.
class LogHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument)
    Q_PROPERTY(QColor timestampColor READ timestampColor WRITE setTimestampColor)
    Q_PROPERTY(QColor txColor READ txColor WRITE setTxColor)
    Q_PROPERTY(QColor rxColor READ rxColor WRITE setRxColor)
    Q_PROPERTY(QColor sysColor READ sysColor WRITE setSysColor)
    Q_PROPERTY(QColor errColor READ errColor WRITE setErrColor)

public:
    explicit LogHighlighter(QObject *parent = nullptr);

    QQuickTextDocument *document() const { return quickDocument_; }
    void setDocument(QQuickTextDocument *document);

    QColor timestampColor() const { return timestampColor_; }
    void setTimestampColor(const QColor &color);
    QColor txColor() const { return txColor_; }
    void setTxColor(const QColor &color);
    QColor rxColor() const { return rxColor_; }
    void setRxColor(const QColor &color);
    QColor sysColor() const { return sysColor_; }
    void setSysColor(const QColor &color);
    QColor errColor() const { return errColor_; }
    void setErrColor(const QColor &color);

protected:
    void highlightBlock(const QString &text) override;

private:
    // Invalid QColor (the default) for anything that isn't one of the four
    // known tags -- highlightBlock() takes that as "not a line this
    // recognizes, leave it alone" rather than forcing some fallback color.
    QColor colorForTag(QStringView tag) const;

    QQuickTextDocument *quickDocument_ = nullptr;
    QColor timestampColor_;
    QColor txColor_;
    QColor rxColor_;
    QColor sysColor_;
    QColor errColor_;
};
