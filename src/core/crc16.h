#pragma once

#include <QByteArray>
#include <QtGlobal>

// CRC16/MODBUS -- poly 0xA001 (the bit-reflected form of 0x8005), init
// 0xFFFF, no final XOR. This is the specific variant the "Append CRC"
// transmit option (SerialSettingsPanel.qml / AppController::crcEnabled)
// uses; it's the standard checksum for Modbus RTU frames and is what most
// AT/industrial serial devices mean by "CRC16" absent other qualification.
// No QML_ELEMENT here -- this is a plain C++ utility AppController computes
// with internally, never something QML calls directly.
namespace Crc16 {

quint16 modbus(const QByteArray &data);

// The 2 bytes to append to a frame for data's CRC16/MODBUS value, in
// Modbus RTU's own on-wire order: low byte first, then high byte.
QByteArray modbusBytes(const QByteArray &data);

}  // namespace Crc16
