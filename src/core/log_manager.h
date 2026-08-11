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
    void entryAdded(const LogEntry &entry);
    void cleared();
    void countersChanged(qint64 rxBytes, qint64 txBytes);

private:
    void writeContinuous(const LogEntry &entry);
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
