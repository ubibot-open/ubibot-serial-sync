#include "core/log_manager.h"

#include <QDir>
#include <QFileInfo>

LogManager::LogManager(QObject *parent) : QObject(parent) {}

QString LogManager::dirTag(LogKind k) {
    switch (k) {
    case LogKind::Tx: return QStringLiteral("TX");
    case LogKind::Rx: return QStringLiteral("RX");
    case LogKind::Sys: return QStringLiteral("SYS");
    case LogKind::Err: return QStringLiteral("ERR");
    }
    return QStringLiteral("?");
}

void LogManager::append(LogKind kind, const QByteArray &data) { appendBatch(kind, {data}); }

void LogManager::appendBatch(LogKind kind, const QList<QByteArray> &lines) {
    if (lines.isEmpty()) return;

    // One timestamp for the whole batch rather than one QDateTime::
    // currentDateTime() call per line -- indistinguishable to a human
    // reading a burst of hundreds of lines that all arrived within the
    // same event-loop turn, and cheaper for a very large one.
    const QDateTime now = QDateTime::currentDateTime();

    QList<LogEntry> batch;
    batch.reserve(lines.size());
    qint64 addedBytes = 0;
    for (const QByteArray &data : lines) {
        batch.push_back(LogEntry{now, kind, data});
        addedBytes += data.size();
    }

    if (kind == LogKind::Rx) rxBytes_ += addedBytes;
    else if (kind == LogKind::Tx) txBytes_ += addedBytes;

    // Continuous file logging mirrors every line actually received,
    // regardless of how many of them go on to be kept in the in-memory
    // scrollback below -- unlike that scrollback, the log file isn't
    // capped at capacity_.
    if (continuousEnabled_) {
        for (const LogEntry &e : batch) writeContinuous(e, /*flush=*/false);
        if (continuousFile_.isOpen()) continuousStream_.flush();
    }

    // A batch bigger than the whole scrollback capacity -- exactly what a
    // device dumping hundreds of thousands of lines in one go looks like --
    // means every line before this batch's own trailing `capacity_` would
    // just be inserted and evicted again immediately without ever being
    // visible. Skip storing (and LogListModel rendering) those outright
    // rather than doing, and immediately discarding, that work.
    const int keepFrom = batch.size() > capacity_ ? batch.size() - capacity_ : 0;
    const int toKeep = batch.size() - keepFrom;

    const int overflow = entries_.size() + toKeep - capacity_;
    const int evictCount = overflow > 0 ? qMin(overflow, entries_.size()) : 0;

    // Batching turned "one full model-update + document-edit cycle per
    // line" into "...per appendBatch() call" -- a huge win when a burst
    // arrives as many small dataReceived chunks (the realistic case,
    // naturally paced by the wire's baud rate), since each chunk is cheap
    // regardless. It does NOT help when a single chunk's own added/evicted
    // count is already large (the OS having buffered a lot before this got
    // a chance to read it, say, or a very high baud rate): the data
    // monitor's TextEdit still has to insert/remove that many lines, and
    // that cost scales with lines touched, not number of calls, even now
    // that the document is plain text rather than per-line HTML (see
    // DataMonitorView.qml/LogHighlighter for why it's plain text at all).
    // Past this many touched lines in one go, rebuilding the whole
    // (capacity_-capped, so bounded regardless of batch size) document
    // from scratch is cheaper than editing it incrementally -- see
    // LogListModel's bulkChanged handling.
    static constexpr int kBulkThreshold = 300;
    const bool bulk = (evictCount + toKeep) > kBulkThreshold;

    if (evictCount > 0) {
        if (!bulk) emit entriesAboutToBeEvicted(evictCount);
        entries_.erase(entries_.begin(), entries_.begin() + evictCount);
        if (!bulk) emit entriesEvicted();
    }

    entries_.reserve(entries_.size() + toKeep);
    for (int i = keepFrom; i < batch.size(); ++i) entries_.push_back(batch.at(i));

    if (bulk) emit bulkChanged();
    else emit entriesAppended(toKeep);
    emit countersChanged(rxBytes_, txBytes_);
}

void LogManager::clear() {
    entries_.clear();
    rxBytes_ = 0;
    txBytes_ = 0;
    emit cleared();
    emit countersChanged(rxBytes_, txBytes_);
}

QString LogManager::formatLine(const LogEntry &e, LogFileFormat format) {
    const QString ts = e.time.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
    const QString dir = dirTag(e.kind);
    switch (format) {
    case LogFileFormat::PlainText:
        return QStringLiteral("[%1] %2  %3").arg(ts, dir, e.asciiText());
    case LogFileFormat::Csv: {
        QString text = e.asciiText();
        text.replace(QLatin1Char('"'), QStringLiteral("\"\""));
        return QStringLiteral("\"%1\",\"%2\",\"%3\"").arg(ts, dir, text);
    }
    case LogFileFormat::HexDump:
        return QStringLiteral("[%1] %2  %3").arg(ts, dir, e.hexText());
    }
    return QString();
}

bool LogManager::exportToFile(const QString &path, LogFileFormat format, QString *error) const {
    const QString dir = QFileInfo(path).absolutePath();
    if (!dir.isEmpty()) QDir().mkpath(dir);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (error) *error = file.errorString();
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    if (format == LogFileFormat::Csv) out << "time,direction,data\n";

    for (const LogEntry &e : entries_) out << formatLine(e, format) << '\n';

    file.close();
    return true;
}

void LogManager::setContinuousLogging(bool enabled, const QString &dir, const QString &baseName) {
    continuousEnabled_ = enabled;
    continuousDir_ = dir;
    continuousBaseName_ = baseName.isEmpty() ? QStringLiteral("ubibot") : baseName;

    if (continuousFile_.isOpen()) {
        continuousStream_.setDevice(nullptr);
        continuousFile_.close();
    }
    continuousDate_ = QDate();

    if (enabled && !continuousDir_.isEmpty()) {
        QDir().mkpath(continuousDir_);
        rotateContinuousFile(QDate::currentDate());
    }
}

void LogManager::rotateContinuousFile(const QDate &date) {
    if (continuousFile_.isOpen()) {
        continuousStream_.setDevice(nullptr);
        continuousFile_.close();
    }

    const QString fileName = QStringLiteral("%1-%2.log")
                                  .arg(continuousBaseName_, date.toString(QStringLiteral("yyyyMMdd")));
    continuousFile_.setFileName(QDir(continuousDir_).filePath(fileName));
    if (continuousFile_.open(QIODevice::Append | QIODevice::Text)) {
        continuousStream_.setDevice(&continuousFile_);
        continuousStream_.setEncoding(QStringConverter::Utf8);
        continuousDate_ = date;
    }
}

void LogManager::writeContinuous(const LogEntry &e, bool flush) {
    const QDate today = e.time.date();
    if (today != continuousDate_) rotateContinuousFile(today);
    if (continuousFile_.isOpen()) {
        continuousStream_ << formatLine(e, LogFileFormat::PlainText) << '\n';
        if (flush) continuousStream_.flush();
    }
}
