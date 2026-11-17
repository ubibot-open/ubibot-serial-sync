// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <QByteArray>
#include <QString>

// User-typed HEX <-> bytes, and converting a send box's content when the
// user switches between ASCII and HEX (the main send box and batch steps
// share these rules).
namespace HexText {

// Pairs of hex digits, optionally separated by whitespace, ',', ';', ':'
// or '-', each pair optionally prefixed with 0x. Anything else (or an odd
// number of digits in a token) makes this return false and leaves *out
// untouched -- unlike QByteArray::fromHex(), which silently skips invalid
// characters (that turned {"command":"ReadProduct"} into CA DE AD DC).
bool parse(const QString &text, QByteArray *out);

// "7B 22 0D 0A" (upper case, space separated).
QString format(const QByteArray &bytes);

// ASCII -> HEX for the send box: the bytes an ASCII send would put on the
// wire (CRC aside), i.e. the UTF-8 text plus the "\r\n" an ASCII send
// appends when the text doesn't already end with '\n'.
QString fromAsciiDraft(const QString &text);

enum class ToAsciiResult { Ok, InvalidHex, NotText };

// HEX -> ASCII for the send box: the inverse of fromAsciiDraft(). A
// trailing "\r\n" is dropped because an ASCII send appends it again.
// NotText when the bytes aren't valid UTF-8 or contain control characters
// other than tab / CR / LF (e.g. AA 55 01), so they can't be edited as text.
ToAsciiResult toAsciiDraft(const QString &hex, QString *out);

}  // namespace HexText
