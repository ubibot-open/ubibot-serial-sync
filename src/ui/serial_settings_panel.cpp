#include "ui/serial_settings_panel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

SerialSettingsPanel::SerialSettingsPanel(QWidget *parent) : QWidget(parent) {
    buildUi();
    populatePorts();
    retranslateUi();
}

void SerialSettingsPanel::buildUi() {
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget;
    auto *root = new QVBoxLayout(content);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(16);

    // --- Port group ---------------------------------------------------
    auto *portGroup = new QGroupBox;
    auto *portForm = new QFormLayout(portGroup);

    auto *portRow = new QWidget;
    auto *portRowLayout = new QHBoxLayout(portRow);
    portRowLayout->setContentsMargins(0, 0, 0, 0);
    portCombo_ = new QComboBox;
    portCombo_->setEditable(true);
    refreshButton_ = new QPushButton;
    refreshButton_->setIcon(QIcon(QStringLiteral(":/icons/refresh.svg")));
    refreshButton_->setFixedWidth(30);
    connect(refreshButton_, &QPushButton::clicked, this, [this] {
        populatePorts();
        emit refreshPortsRequested();
    });
    portRowLayout->addWidget(portCombo_, 1);
    portRowLayout->addWidget(refreshButton_);

    baudCombo_ = new QComboBox;
    baudCombo_->setEditable(true);
    for (qint32 b : SerialManager::standardBaudRates()) baudCombo_->addItem(QString::number(b));
    baudCombo_->setCurrentText(QStringLiteral("115200"));

    dataBitsCombo_ = new QComboBox;
    dataBitsCombo_->addItems({"5", "6", "7", "8"});
    dataBitsCombo_->setCurrentText(QStringLiteral("8"));

    parityCombo_ = new QComboBox;  // filled in retranslateUi (labels are language-dependent)
    stopBitsCombo_ = new QComboBox;
    stopBitsCombo_->addItems({"1", "1.5", "2"});

    flowControlCombo_ = new QComboBox;  // filled in retranslateUi

    portLabel_ = new QLabel;
    baudLabel_ = new QLabel;
    dataBitsLabel_ = new QLabel;
    parityLabel_ = new QLabel;
    stopBitsLabel_ = new QLabel;
    flowControlLabel_ = new QLabel;

    portForm->addRow(portLabel_, portRow);
    portForm->addRow(baudLabel_, baudCombo_);
    portForm->addRow(dataBitsLabel_, dataBitsCombo_);
    portForm->addRow(parityLabel_, parityCombo_);
    portForm->addRow(stopBitsLabel_, stopBitsCombo_);
    portForm->addRow(flowControlLabel_, flowControlCombo_);

    // --- Receive group --------------------------------------------------
    auto *rxGroup = new QGroupBox;
    auto *rxLayout = new QVBoxLayout(rxGroup);
    auto *rxRadioRow = new QHBoxLayout;
    rxAsciiRadio_ = new QRadioButton(QStringLiteral("ASCII"));
    rxHexRadio_ = new QRadioButton(QStringLiteral("HEX"));
    rxAsciiRadio_->setChecked(true);
    rxRadioRow->addWidget(rxAsciiRadio_);
    rxRadioRow->addWidget(rxHexRadio_);
    rxRadioRow->addStretch();
    connect(rxHexRadio_, &QRadioButton::toggled, this, [this](bool on) { emit receiveFormatChanged(on); });

    timestampCheck_ = new QCheckBox;
    timestampCheck_->setChecked(true);
    connect(timestampCheck_, &QCheckBox::toggled, this, &SerialSettingsPanel::showTimestampChanged);
    wrapCheck_ = new QCheckBox;
    wrapCheck_->setChecked(true);
    echoTxCheck_ = new QCheckBox;
    echoTxCheck_->setChecked(true);

    rxLayout->addLayout(rxRadioRow);
    rxLayout->addWidget(timestampCheck_);
    rxLayout->addWidget(wrapCheck_);
    rxLayout->addWidget(echoTxCheck_);

    // --- Transmit group --------------------------------------------------
    auto *txGroup = new QGroupBox;
    auto *txLayout = new QVBoxLayout(txGroup);
    auto *txRadioRow = new QHBoxLayout;
    txAsciiRadio_ = new QRadioButton(QStringLiteral("ASCII"));
    txHexRadio_ = new QRadioButton(QStringLiteral("HEX"));
    txAsciiRadio_->setChecked(true);
    txRadioRow->addWidget(txAsciiRadio_);
    txRadioRow->addWidget(txHexRadio_);
    txRadioRow->addStretch();

    auto *repeatRow = new QHBoxLayout;
    repeatCheck_ = new QCheckBox;
    repeatIntervalSpin_ = new QSpinBox;
    repeatIntervalSpin_->setRange(50, 3'600'000);
    repeatIntervalSpin_->setValue(1000);
    repeatIntervalSpin_->setEnabled(false);
    repeatUnitLabel_ = new QLabel;
    connect(repeatCheck_, &QCheckBox::toggled, this, [this](bool on) {
        repeatIntervalSpin_->setEnabled(on);
        emit repeatSendToggled(on, repeatIntervalSpin_->value());
    });
    connect(repeatIntervalSpin_, &QSpinBox::valueChanged, this, [this](int v) {
        if (repeatCheck_->isChecked()) emit repeatSendToggled(true, v);
    });
    repeatRow->addWidget(repeatCheck_);
    repeatRow->addWidget(repeatIntervalSpin_);
    repeatRow->addWidget(repeatUnitLabel_);
    repeatRow->addStretch();

    txLayout->addLayout(txRadioRow);
    txLayout->addLayout(repeatRow);

    root->addWidget(portGroup);
    root->addWidget(rxGroup);
    root->addWidget(txGroup);
    root->addStretch();

    scroll->setWidget(content);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    portGroupBox_ = portGroup;
    rxGroupBox_ = rxGroup;
    txGroupBox_ = txGroup;
}

void SerialSettingsPanel::populatePorts() {
    const QString previous = portCombo_->currentText();
    portCombo_->clear();
    for (const auto &info : SerialManager::availablePorts()) {
        QString label = info.portName;
        if (!info.description.isEmpty()) label += QStringLiteral(" (%1)").arg(info.description);
        portCombo_->addItem(label, info.portName);
    }
    if (!previous.isEmpty()) portCombo_->setCurrentText(previous);
}

void SerialSettingsPanel::retranslateUi() {
    portGroupBox_->setTitle(tr("Port"));
    rxGroupBox_->setTitle(tr("Receive"));
    txGroupBox_->setTitle(tr("Transmit"));

    portLabel_->setText(tr("Port"));
    baudLabel_->setText(tr("Baud rate"));
    dataBitsLabel_->setText(tr("Data bits"));
    parityLabel_->setText(tr("Parity"));
    stopBitsLabel_->setText(tr("Stop bits"));
    flowControlLabel_->setText(tr("Flow control"));

    const int parityIdx = parityCombo_->currentIndex();
    parityCombo_->clear();
    parityCombo_->addItem(tr("None"), int(QSerialPort::NoParity));
    parityCombo_->addItem(tr("Even"), int(QSerialPort::EvenParity));
    parityCombo_->addItem(tr("Odd"), int(QSerialPort::OddParity));
    parityCombo_->setCurrentIndex(parityIdx >= 0 ? parityIdx : 0);

    const int flowIdx = flowControlCombo_->currentIndex();
    flowControlCombo_->clear();
    flowControlCombo_->addItem(tr("None"), int(QSerialPort::NoFlowControl));
    flowControlCombo_->addItem(tr("RTS/CTS"), int(QSerialPort::HardwareControl));
    flowControlCombo_->addItem(tr("XON/XOFF"), int(QSerialPort::SoftwareControl));
    flowControlCombo_->setCurrentIndex(flowIdx >= 0 ? flowIdx : 0);

    timestampCheck_->setText(tr("Show timestamp"));
    wrapCheck_->setText(tr("Wrap lines"));
    echoTxCheck_->setText(tr("Echo sent data"));

    repeatCheck_->setText(tr("Repeat send"));
    repeatUnitLabel_->setText(tr("ms"));
}

void SerialSettingsPanel::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}

SerialConfig SerialSettingsPanel::currentConfig() const {
    SerialConfig cfg;
    cfg.portName = portCombo_->currentData().isValid() ? portCombo_->currentData().toString()
                                                         : portCombo_->currentText();
    cfg.baudRate = baudCombo_->currentText().toInt();
    cfg.dataBits = static_cast<QSerialPort::DataBits>(dataBitsCombo_->currentText().toInt());
    cfg.parity = static_cast<QSerialPort::Parity>(parityCombo_->currentData().toInt());
    cfg.stopBits = stopBitsCombo_->currentText() == QStringLiteral("1.5")
                       ? QSerialPort::OneAndHalfStop
                   : stopBitsCombo_->currentText() == QStringLiteral("2") ? QSerialPort::TwoStop
                                                                           : QSerialPort::OneStop;
    cfg.flowControl = static_cast<QSerialPort::FlowControl>(flowControlCombo_->currentData().toInt());
    return cfg;
}

void SerialSettingsPanel::setConfig(const SerialConfig &cfg) {
    if (!cfg.portName.isEmpty()) portCombo_->setCurrentText(cfg.portName);
    baudCombo_->setCurrentText(QString::number(cfg.baudRate));
    dataBitsCombo_->setCurrentText(QString::number(int(cfg.dataBits)));
    parityCombo_->setCurrentIndex(parityCombo_->findData(int(cfg.parity)));
    stopBitsCombo_->setCurrentText(cfg.stopBits == QSerialPort::OneAndHalfStop ? QStringLiteral("1.5")
                                    : cfg.stopBits == QSerialPort::TwoStop     ? QStringLiteral("2")
                                                                                : QStringLiteral("1"));
    flowControlCombo_->setCurrentIndex(flowControlCombo_->findData(int(cfg.flowControl)));
}

bool SerialSettingsPanel::receiveAsHex() const { return rxHexRadio_->isChecked(); }
bool SerialSettingsPanel::showTimestamp() const { return timestampCheck_->isChecked(); }
bool SerialSettingsPanel::wrapLines() const { return wrapCheck_->isChecked(); }
bool SerialSettingsPanel::echoTx() const { return echoTxCheck_->isChecked(); }
bool SerialSettingsPanel::sendAsHex() const { return txHexRadio_->isChecked(); }
bool SerialSettingsPanel::repeatSendEnabled() const { return repeatCheck_->isChecked(); }
int SerialSettingsPanel::repeatIntervalMs() const { return repeatIntervalSpin_->value(); }

void SerialSettingsPanel::setEnabledWhileConnected(bool connected) {
    // Frame settings can't be changed while the port is open; receive/transmit
    // display options remain live-editable.
    const QList<QWidget *> widgets = {portCombo_,    refreshButton_,   baudCombo_,        dataBitsCombo_,
                                       parityCombo_, stopBitsCombo_, flowControlCombo_};
    for (QWidget *w : widgets) w->setEnabled(!connected);
}
