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

void LogManager::append(LogKind kind, const QByteArray &data) {
    LogEntry e;
    e.time = QDateTime::currentDateTime();
    e.kind = kind;
    e.data = data;

    if (entries_.size() >= capacity_) {
        emit rowAboutToBeRemoved();
        entries_.erase(entries_.begin());
        emit rowRemoved();
    }
    entries_.push_back(e);

    if (kind == LogKind::Rx) rxBytes_ += data.size();
    else if (kind == LogKind::Tx) txBytes_ += data.size();

    if (continuousEnabled_) writeContinuous(e);

    emit entryAdded(e);
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

void LogManager::writeContinuous(const LogEntry &e) {
    const QDate today = e.time.date();
    if (today != continuousDate_) rotateContinuousFile(today);
    if (continuousFile_.isOpen()) {
        continuousStream_ << formatLine(e, LogFileFormat::PlainText) << '\n';
        continuousStream_.flush();
    }
}
