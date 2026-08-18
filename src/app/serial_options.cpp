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
    // Selection-only now (no more free-text entry -- see baudCombo's own
    // comment in SerialSettingsPanel.qml), so this needs to actually cover
    // what people use rather than just a handful of common picks: every
    // standard rate from 1200 up to 921600, ascending.
    QVariantList result;
    for (qint32 rate : {300, 600, 900, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 230400, 460800, 921600}) {
        result.push_back(rate);
    }
    return result;
}
