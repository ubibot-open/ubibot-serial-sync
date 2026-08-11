#pragma once

#include "core/serial_manager.h"

#include <QWidget>

class QComboBox;
class QCheckBox;
class QRadioButton;
class QSpinBox;
class QPushButton;
class QLabel;
class QGroupBox;

// Left-hand panel shown in "Serial" mode: port/baud/frame settings plus
// receive/transmit display options. Owns no serial I/O itself -- MainWindow
// reads currentConfig() when the user hits "Open port" and reads the
// receive/transmit toggles on demand.
class SerialSettingsPanel : public QWidget {
    Q_OBJECT
public:
    explicit SerialSettingsPanel(QWidget *parent = nullptr);

    SerialConfig currentConfig() const;
    void setConfig(const SerialConfig &cfg);

    bool receiveAsHex() const;
    bool showTimestamp() const;
    bool wrapLines() const;
    bool echoTx() const;

    bool sendAsHex() const;
    bool repeatSendEnabled() const;
    int repeatIntervalMs() const;

    void setEnabledWhileConnected(bool connected);

signals:
    void refreshPortsRequested();
    void receiveFormatChanged(bool hex);
    void showTimestampChanged(bool show);
    void repeatSendToggled(bool enabled, int intervalMs);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void retranslateUi();
    void populatePorts();

    QComboBox *portCombo_;
    QPushButton *refreshButton_;
    QComboBox *baudCombo_;
    QComboBox *dataBitsCombo_;
    QComboBox *parityCombo_;
    QComboBox *stopBitsCombo_;
    QComboBox *flowControlCombo_;

    QRadioButton *rxAsciiRadio_;
    QRadioButton *rxHexRadio_;
    QCheckBox *timestampCheck_;
    QCheckBox *wrapCheck_;
    QCheckBox *echoTxCheck_;

    QRadioButton *txAsciiRadio_;
    QRadioButton *txHexRadio_;
    QCheckBox *repeatCheck_;
    QSpinBox *repeatIntervalSpin_;

    QGroupBox *portGroupBox_;
    QGroupBox *rxGroupBox_;
    QGroupBox *txGroupBox_;
    QLabel *portLabel_;
    QLabel *baudLabel_;
    QLabel *dataBitsLabel_;
    QLabel *parityLabel_;
    QLabel *stopBitsLabel_;
    QLabel *flowControlLabel_;
    QLabel *repeatUnitLabel_;
};
