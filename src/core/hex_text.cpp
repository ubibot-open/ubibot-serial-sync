// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "core/hex_text.h"

namespace HexText {

namespace {

bool isHexDigit(QChar c) {
    const ushort u = c.unicode();
    return (u >= '0' && u <= '9') || (u >= 'a' && u <= 'f') || (u >= 'A' && u <= 'F');
}

bool isSeparator(QChar c) {
    return c.isSpace() || c == QLatin1Char(',') || c == QLatin1Char(';') || c == QLatin1Char(':') ||
           c == QLatin1Char('-');
}

bool isPlainText(const QByteArray &bytes) {
    const QString text = QString::fromUtf8(bytes);
    if (text.toUtf8() != bytes) return false;
    for (const QChar c : text)
        if (c.category() == QChar::Other_Control && c != QLatin1Char('\t') && c != QLatin1Char('\r') &&
            c != QLatin1Char('\n'))
            return false;
    return true;
}

}  // namespace

bool parse(const QString &text, QByteArray *out) {
    QByteArray bytes;
    QString token;
    const auto flush = [&]() -> bool {
        if (token.isEmpty()) return true;
        if (token.startsWith(QLatin1String("0x"), Qt::CaseInsensitive)) token.remove(0, 2);
        if (token.isEmpty() || token.size() % 2 != 0) return false;
        for (const QChar c : token)
            if (!isHexDigit(c)) return false;
        bytes += QByteArray::fromHex(token.toLatin1());
        token.clear();
        return true;
    };
    for (const QChar c : text) {
        if (isSeparator(c)) {
            if (!flush()) return false;
        } else {
            token += c;
        }
    }
    if (!flush()) return false;
    *out = bytes;
    return true;
}

QString format(const QByteArray &bytes) { return QString::fromLatin1(bytes.toHex(' ').toUpper()); }

QString fromAsciiDraft(const QString &text) {
    QByteArray bytes = text.toUtf8();
    if (!bytes.endsWith('\n')) bytes += "\r\n";
    return format(bytes);
}

ToAsciiResult toAsciiDraft(const QString &hex, QString *out) {
    QByteArray bytes;
    if (!parse(hex, &bytes)) return ToAsciiResult::InvalidHex;
    if (!isPlainText(bytes)) return ToAsciiResult::NotText;
    if (bytes.endsWith("\r\n")) bytes.chop(2);
    *out = QString::fromUtf8(bytes);
    return ToAsciiResult::Ok;
}

}  // namespace HexText
