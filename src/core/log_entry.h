#pragma once

#include <QByteArray>
#include <QDateTime>

// Direction/kind of a single line in the data monitor. Tx/Rx are actual wire
// traffic; Sys is an informational line the app itself injects (port
// opened/closed, wizard finished, ...); Err flags a local problem (e.g. "port
// not open").
enum class LogKind { Tx, Rx, Sys, Err };

struct LogEntry {
    QDateTime time;
    LogKind kind = LogKind::Sys;
    QByteArray data;

    QString asciiText() const { return QString::fromLatin1(data); }
    QString hexText() const {
        return QString::fromLatin1(data.toHex(' ').toUpper());
    }
};
