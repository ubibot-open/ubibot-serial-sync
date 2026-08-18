#pragma once

#include <QObject>
#include <QSerialPort>
#include <QVector>

struct SerialConfig {
    QString portName;
    qint32 baudRate = 115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
};

// Thin, signal/slot-friendly wrapper around QSerialPort. Keeps the rest of
// the app from touching QSerialPort directly so error handling and the
// "current config" bookkeeping live in one place.
class SerialManager : public QObject {
    Q_OBJECT
public:
    // How a detected port should be presented in the port picker:
    // "recommended" for chips UbiBot devices actually ship
    // with (CH340, CP210x), "available" otherwise. (QtSerialPort has no
    // cross-platform way to tell whether a port is already open elsewhere
    // without attempting open() itself, so there's no "busy" hint.)
    enum class PortHint { Recommended, Available };

    struct PortInfo {
        QString portName;
        QString description;
        // Human-readable bridge-chip name ("CH340", "CP210x") when the
        // port's USB vendor ID matches one UbiBot devices actually ship
        // with, empty otherwise. Windows' own `description` is often just
        // a generic "USB Serial Device" that doesn't say which chip it is
        // -- this is what actually tells the two apart in the picker.
        QString chipLabel;
        PortHint hint = PortHint::Available;
    };

    static QVector<PortInfo> availablePorts();
    static QList<qint32> standardBaudRates();

    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager() override;

    bool open(const SerialConfig &cfg, QString *error = nullptr);
    void close();
    bool isOpen() const;

    qint64 write(const QByteArray &data);

    const SerialConfig &currentConfig() const { return cfg_; }
    // e.g. "115200 8-N-1"
    QString portSummary() const;
    // Same formatting as portSummary(), for an arbitrary config rather than
    // this manager's currently-open one -- e.g. previewing what a device
    // model's recommended serial settings would look like before the port
    // is actually opened.
    static QString summaryFor(const SerialConfig &cfg);

signals:
    void dataReceived(const QByteArray &data);
    void opened(const SerialConfig &cfg);
    void closed();
    void errorOccurred(const QString &message);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError err);

private:
    QSerialPort port_;
    SerialConfig cfg_;
};
