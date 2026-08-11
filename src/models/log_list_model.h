#pragma once

#include "core/ansi_text.h"
#include "core/log_manager.h"

#include <QAbstractListModel>
#include <QQmlEngine>

// QML-facing view over LogManager's scrollback. Still a QAbstractListModel
// (one row per LogEntry, `time`/`text`/`html`/`color` roles) for anything
// that wants row-at-a-time access, but DataMonitorView itself no longer
// uses it that way -- a ListView of one row per entry only ever lets the
// user select text within a single row at a time, not drag a selection
// across lines like a real terminal. Instead it renders the whole
// scrollback as ONE continuous rich-text document (single TextEdit) built
// from lineAppended()/lineEvicted()/rebuildNeeded() below, which is also
// why those exist alongside the per-row API: appending is O(1) (a
// TextEdit.insert at the end) rather than re-binding the whole document's
// text on every new line.
class LogListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by AppController")

    Q_PROPERTY(bool hexMode READ hexMode WRITE setHexMode NOTIFY hexModeChanged)
    Q_PROPERTY(bool showTimestamp READ showTimestamp WRITE setShowTimestamp NOTIFY showTimestampChanged)
    Q_PROPERTY(int lineCount READ lineCount NOTIFY lineCountChanged)
    Q_PROPERTY(qint64 rxBytes READ rxBytes NOTIFY countersChanged)
    Q_PROPERTY(qint64 txBytes READ txBytes NOTIFY countersChanged)

public:
    enum Roles { TimeRole = Qt::UserRole + 1, DirRole, TextRole, ColorRole, HtmlRole };

    explicit LogListModel(LogManager *manager, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool hexMode() const { return hexMode_; }
    void setHexMode(bool hex);
    bool showTimestamp() const { return showTimestamp_; }
    void setShowTimestamp(bool show);
    int lineCount() const;
    qint64 rxBytes() const;
    qint64 txBytes() const;

    Q_INVOKABLE void clear();

    // Every entry currently in scope, pre-rendered exactly like
    // lineAppended() below and concatenated in order -- what
    // DataMonitorView sets as its TextEdit's initial `text`, and rebuilds
    // wholesale from on rebuildNeeded() (hexMode/showTimestamp changed, or
    // the log was cleared). Each line's HTML already includes its own
    // trailing "<br/>", including the last one, so a line's rendered
    // length (see lineHtml() below) stays valid uniformly whether it was
    // there from this dump or arrived later via lineAppended().
    Q_INVOKABLE QString fullHtmlDump() const;

signals:
    void hexModeChanged();
    void showTimestampChanged();
    void lineCountChanged();
    void countersChanged();

    // DataMonitorView's single continuous document is maintained
    // incrementally from these rather than re-bound to the whole
    // scrollback on every change, which would re-layout the entire
    // document (thousands of lines, worst case) on every single new one.
    void lineAppended(const QString &html);
    // docLength is how many characters (as TextEdit counts them -- visible
    // text only, no markup, "<br/>" itself counting as exactly one) the
    // evicted line + its trailing separator occupy at the very front of
    // the document, i.e. exactly what a `contentEdit.remove(0, docLength)`
    // needs to drop it and nothing more.
    void lineEvicted(int docLength);
    void rebuildNeeded();

private:
    // Result.length is the line's rendered length (prefix + content),
    // exactly what lineEvicted()'s docLength needs -- see ansi_text.h.
    AnsiText::Result lineHtml(const LogEntry &e) const;

    LogManager *manager_;
    bool hexMode_ = false;
    bool showTimestamp_ = true;
};
