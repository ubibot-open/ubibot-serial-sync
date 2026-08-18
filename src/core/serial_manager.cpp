#include "core/serial_manager.h"

#include <QSerialPortInfo>

SerialManager::SerialManager(QObject *parent) : QObject(parent) {
    connect(&port_, &QSerialPort::readyRead, this, &SerialManager::handleReadyRead);
    connect(&port_, &QSerialPort::errorOccurred, this, &SerialManager::handleError);
}

SerialManager::~SerialManager() {
    close();
}

QVector<SerialManager::PortInfo> SerialManager::availablePorts() {
    QVector<PortInfo> result;
    const auto ports = QSerialPortInfo::availablePorts();
    result.reserve(ports.size());
    for (const QSerialPortInfo &info : ports) {
        PortInfo pi;
        pi.portName = info.portName();
        pi.description = info.description();

        // UbiBot's USB-serial adapters are built around the WCH CH340
        // (VID 0x1A86) and Silicon Labs CP210x (VID 0x10C4) bridge chips;
        // flag those as the likely-correct pick in the port picker.
        // (QtSerialPort has no cross-platform "is this port already open
        // elsewhere" query -- the only way to find out is to attempt
        // open(), which has side effects -- so there's no "Busy" detection
        // here.)
        if (info.hasVendorIdentifier() && info.vendorIdentifier() == 0x1A86) {
            pi.chipLabel = QStringLiteral("CH340");
        } else if (info.hasVendorIdentifier() && info.vendorIdentifier() == 0x10C4) {
            pi.chipLabel = QStringLiteral("CP210x");
        }
        pi.hint = pi.chipLabel.isEmpty() ? PortHint::Available : PortHint::Recommended;
        result.push_back(pi);
    }
    return result;
}

QList<qint32> SerialManager::standardBaudRates() {
    return {9600, 19200, 57600, 115200, 230400};
}

bool SerialManager::open(const SerialConfig &cfg, QString *error) {
    close();

    port_.setPortName(cfg.portName);
    port_.setBaudRate(cfg.baudRate);
    port_.setDataBits(cfg.dataBits);
    port_.setParity(cfg.parity);
    port_.setStopBits(cfg.stopBits);
    port_.setFlowControl(cfg.flowControl);

    if (!port_.open(QIODevice::ReadWrite)) {
        if (error) *error = port_.errorString();
        return false;
    }

    cfg_ = cfg;
    emit opened(cfg_);
    return true;
}

void SerialManager::close() {
    if (port_.isOpen()) {
        port_.close();
        emit closed();
    }
}

bool SerialManager::isOpen() const {
    return port_.isOpen();
}

qint64 SerialManager::write(const QByteArray &data) {
    if (!port_.isOpen()) return -1;
    return port_.write(data);
}

QString SerialManager::portSummary() const { return summaryFor(cfg_); }

QString SerialManager::summaryFor(const SerialConfig &cfg) {
    auto parityChar = [](QSerialPort::Parity p) {
        switch (p) {
        case QSerialPort::NoParity: return QStringLiteral("N");
        case QSerialPort::EvenParity: return QStringLiteral("E");
        case QSerialPort::OddParity: return QStringLiteral("O");
        case QSerialPort::SpaceParity: return QStringLiteral("S");
        case QSerialPort::MarkParity: return QStringLiteral("M");
        default: return QStringLiteral("?");
        }
    };
    auto stopBitsText = [](QSerialPort::StopBits s) {
        switch (s) {
        case QSerialPort::OneStop: return QStringLiteral("1");
        case QSerialPort::OneAndHalfStop: return QStringLiteral("1.5");
        case QSerialPort::TwoStop: return QStringLiteral("2");
        default: return QStringLiteral("?");
        }
    };
    return QStringLiteral("%1 %2-%3-%4")
        .arg(cfg.baudRate)
        .arg(int(cfg.dataBits))
        .arg(parityChar(cfg.parity), stopBitsText(cfg.stopBits));
}

void SerialManager::handleReadyRead() {
    emit dataReceived(port_.readAll());
}

void SerialManager::handleError(QSerialPort::SerialPortError err) {
    if (err == QSerialPort::NoError) return;

    // QSerialPort has no dedicated "device physically removed" signal --
    // unplugging a USB-serial adapter while its port is open just makes the
    // next read/write fail, and on Windows that most often comes back as
    // PermissionError ("Access is denied", which is misleading here since
    // nothing about permissions changed) rather than ResourceError.
    // DeviceNotFoundError covers the equivalent case on Linux/macOS or on a
    // later open() attempt. Whichever of the three shows up, it means the
    // port is simply gone, not that something is wrong with how this app is
    // talking to it -- report that plainly instead of surfacing the raw,
    // platform-specific OS string, and close our side of the connection so
    // the toolbar's Open/Close port button (bound to isOpen()) doesn't
    // stay stuck on "Close port" for a port that no longer exists to close.
    const bool disconnected = err == QSerialPort::ResourceError || err == QSerialPort::PermissionError ||
                               err == QSerialPort::DeviceNotFoundError;
    if (disconnected && port_.isOpen()) {
        emit errorOccurred(tr("Serial port disconnected."));
        close();
        return;
    }

    emit errorOccurred(port_.errorString());
}
