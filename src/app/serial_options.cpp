#include "app/serial_options.h"

#include <QSerialPort>

namespace {
QVariantMap opt(const QString &label, int value) {
    QVariantMap m;
    m[QStringLiteral("label")] = label;
    m[QStringLiteral("value")] = value;
    return m;
}
}  // namespace

QVariantList SerialOptions::dataBitsOptions() const {
    return {
        opt(QStringLiteral("5"), QSerialPort::Data5),
        opt(QStringLiteral("6"), QSerialPort::Data6),
        opt(QStringLiteral("7"), QSerialPort::Data7),
        opt(QStringLiteral("8"), QSerialPort::Data8),
    };
}

QVariantList SerialOptions::parityOptions() const {
    return {
        opt(tr("None"), QSerialPort::NoParity),
        opt(tr("Even"), QSerialPort::EvenParity),
        opt(tr("Odd"), QSerialPort::OddParity),
    };
}

QVariantList SerialOptions::stopBitsOptions() const {
    return {
        opt(QStringLiteral("1"), QSerialPort::OneStop),
        opt(QStringLiteral("1.5"), QSerialPort::OneAndHalfStop),
        opt(QStringLiteral("2"), QSerialPort::TwoStop),
    };
}

QVariantList SerialOptions::flowControlOptions() const {
    return {
        opt(tr("None"), QSerialPort::NoFlowControl),
        opt(tr("RTS/CTS"), QSerialPort::HardwareControl),
        opt(tr("XON/XOFF"), QSerialPort::SoftwareControl),
    };
}

QVariantList SerialOptions::baudRateOptions() const {
    QVariantList result;
    for (qint32 rate : {9600, 19200, 57600, 115200, 230400}) result.push_back(rate);
    return result;
}
