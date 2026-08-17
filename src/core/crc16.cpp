#include "core/crc16.h"

quint16 Crc16::modbus(const QByteArray &data) {
    quint16 crc = 0xFFFF;
    for (const unsigned char byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

QByteArray Crc16::modbusBytes(const QByteArray &data) {
    const quint16 crc = modbus(data);
    QByteArray out;
    out.append(char(crc & 0xFF));
    out.append(char((crc >> 8) & 0xFF));
    return out;
}
