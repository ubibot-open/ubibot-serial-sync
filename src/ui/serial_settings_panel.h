// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <QScrollArea>

class AppController;
class SerialOptions;

namespace QLight {
class CheckBox;

class SpinBox;
class ToolButton;
}  // namespace QLight

namespace ui {

class ComboBox;
class PortToggleButton;
class SegmentedControl;
class ThemedLabel;

// "Serial" tab of the left panel: port parameters, open/close, receive and
// transmit options.
class SerialSettingsPanel : public QScrollArea {
    Q_OBJECT
public:
    explicit SerialSettingsPanel(AppController *controller, QWidget *parent = nullptr);

    bool wrapLines() const;

signals:
    void wrapLinesChanged(bool wrap);

protected:
    void changeEvent(QEvent *e) override;

private:
    void retranslateUi();
    void rebuildOptionCombos();
    void syncPortCombo();
    void applyModelSerialDefaults();
    void updateConnectionState();
    void togglePort();

    AppController *controller_;
    SerialOptions *options_;

    ThemedLabel *portHeading_;
    ThemedLabel *portLabel_;
    ThemedLabel *baudLabel_;
    ThemedLabel *dataBitsLabel_;
    ThemedLabel *parityLabel_;
    ThemedLabel *stopBitsLabel_;
    ThemedLabel *flowLabel_;
    ComboBox *portCombo_;
    QLight::ToolButton *refreshButton_;
    ComboBox *baudCombo_;
    ComboBox *dataBitsCombo_;
    ComboBox *parityCombo_;
    ComboBox *stopBitsCombo_;
    ComboBox *flowCombo_;
    PortToggleButton *openButton_;

    ThemedLabel *receiveHeading_;
    SegmentedControl *receiveMode_;
    QLight::CheckBox *timestampCheck_;
    QLight::CheckBox *wrapCheck_;
    QLight::CheckBox *echoCheck_;

    ThemedLabel *transmitHeading_;
    SegmentedControl *transmitMode_;
    QLight::CheckBox *crcCheck_;
    QLight::CheckBox *repeatCheck_;
    QLight::SpinBox *repeatSpin_;
    ThemedLabel *msLabel_;

    bool syncingPorts_ = false;
};

}  // namespace ui
