#pragma once

#include <QByteArray>
#include <QDateTime>

// Direction/kind of a single line in the data monitor. Tx/Rx are actual wire
// traffic; Sys is an informational line the app itself injects (port
// opened/closed, ...); Err flags a local problem (e.g. "port not open").
enum class LogKind { Tx, Rx, Sys, Err };

struct LogEntry {
    QDateTime time;
    LogKind kind = LogKind::Sys;
    QByteArray data;

    // fromUtf8, not fromLatin1 -- every locally-generated line (TX echo,
    // SYS/ERR messages) is encoded with QString::toUtf8() in
    // AppController, and a localized ERR/SYS message in a non-English
    // language is multi-byte UTF-8. Decoding those bytes as Latin-1 instead
    // turned each multi-byte character into several garbled Latin-1 ones
    // (invisible for plain ASCII text, which is why this only showed up
    // once a Chinese error/status message hit the log). Real RX bytes off
    // the wire are usually plain ASCII too, a subset of UTF-8, so this
    // doesn't change how those render; a device that sends something that
    // isn't valid UTF-8 gets Unicode replacement characters here instead of
    // silently-wrong text -- HEX mode (hexText() below) is the right tool
    // for genuinely binary payloads anyway.
    QString asciiText() const { return QString::fromUtf8(data); }
    QString hexText() const {
        return QString::fromLatin1(data.toHex(' ').toUpper());
    }
};
