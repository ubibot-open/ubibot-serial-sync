#pragma once

#include "core/log_entry.h"

#include <QDate>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QVector>

// Format used both by SaveLogDialog's one-shot export and by continuous
// (auto-rotating) file logging.
enum class LogFileFormat { PlainText, Csv, HexDump };

// Owns the in-memory scrollback for the data monitor plus RX/TX byte
// counters, and optionally mirrors every entry to disk in real time with a
// daily-rotating file (see setContinuousLogging()).
class LogManager : public QObject {
    Q_OBJECT
public:
    explicit LogManager(QObject *parent = nullptr);

    void append(LogKind kind, const QByteArray &data);
    // Same as calling append() once per line, but as ONE mutation + ONE set
    // of signals instead of `lines.size()` of each -- see the appendBatch()
    // comment below for why that difference matters. AppController's RX
    // line-splitter is the only caller that can realistically hand this
    // more than a handful of lines at once (a device dumping a large stored
    // log all at once, say), but nothing stops any other bursty caller from
    // using it too.
    void appendBatch(LogKind kind, const QList<QByteArray> &lines);
    void clear();

    const QVector<LogEntry> &entries() const { return entries_; }
    qint64 rxBytes() const { return rxBytes_; }
    qint64 txBytes() const { return txBytes_; }

    // One-shot export of everything currently in memory.
    bool exportToFile(const QString &path, LogFileFormat format, QString *error = nullptr) const;

    // When enabled, every appended entry is also written to
    // "<dir>/<baseName>-YYYYMMDD.log" (plain text), rolling to a new file the
    // first time an entry lands on a new local date.
    void setContinuousLogging(bool enabled, const QString &dir = QString(),
                               const QString &baseName = QString());
    bool continuousLoggingEnabled() const { return continuousEnabled_; }

signals:
    void cleared();
    void countersChanged(qint64 rxBytes, qint64 txBytes);

    // `count` new entries were just pushed onto the back of entries() --
    // i.e. entries()[entries().size() - count .. entries().size() - 1].
    // Was a single entryAdded(const LogEntry &) fired once per line; a
    // device firehosing hundreds of thousands of lines in one go (a stored
    // log dumped over serial, say) turned that into hundreds of thousands
    // of separate QAbstractListModel row-insert cycles AND hundreds of
    // thousands of separate single-line inserts into the data monitor's
    // TextEdit document -- each individually cheap, but a
    // QTextDocument::remove() at position 0 (see the eviction signals
    // below) is not O(1) in document size, and paying that cost once per
    // line rather than once per *batch* is what actually froze the UI.
    // LogListModel batches every kept line's rich text into one string and
    // fires one lineAppended() for the lot, rather than once per entry.
    void entriesAppended(int count);

    // Bracket the removal of entries()[0 .. count-1] when appendBatch()
    // trims the scrollback to capacity_, so a QAbstractListModel view
    // (LogListModel) can call beginRemoveRows()/endRemoveRows() around the
    // actual mutation instead of guessing, and so it can compute the
    // combined rendered length of every evicted line while they're still
    // present (entriesAboutToBeEvicted fires before the erase; the
    // corresponding entriesAppended for whatever's replacing them always
    // follows, same as the old single-entry version did).
    void entriesAboutToBeEvicted(int count);
    void entriesEvicted();

    // Fired instead of the incremental entriesAppended/entriesAboutToBeEvicted
    // pair above when a single appendBatch() touches enough lines (added
    // and/or evicted) that rendering each individually would cost more than
    // just throwing away the data monitor's whole document and rebuilding it
    // fresh from entries() (already capped at capacity_, so that rebuild's
    // cost doesn't grow with how big the batch was). See LogListModel and
    // appendBatch()'s kBulkThreshold.
    void bulkChanged();

private:
    // `flush` is false when called from a loop that will flush once itself
    // after -- appendBatch() writing a few hundred thousand lines to the
    // continuous log file one fsync-ing QTextStream::flush() at a time
    // would trade the freeze this whole change is fixing for a slower one.
    void writeContinuous(const LogEntry &entry, bool flush = true);
    void rotateContinuousFile(const QDate &date);
    static QString formatLine(const LogEntry &e, LogFileFormat format);
    static QString dirTag(LogKind k);

    QVector<LogEntry> entries_;
    int capacity_ = 5000;
    qint64 rxBytes_ = 0;
    qint64 txBytes_ = 0;

    bool continuousEnabled_ = false;
    QString continuousDir_;
    QString continuousBaseName_;
    QDate continuousDate_;
    QFile continuousFile_;
    QTextStream continuousStream_;
};
