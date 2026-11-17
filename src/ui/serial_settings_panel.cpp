// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/serial_settings_panel.h"

#include "app/app_controller.h"
#include "app/serial_options.h"
#include "ui/ui_common.h"

#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QLight/CheckBox.h>
#include <QLight/FluentIcon.h>
#include <QLight/ScrollBar.h>
#include <QLight/SpinBox.h>
#include <QLight/ToolButton.h>

namespace ui {

namespace {

constexpr int kDefaultBaud = 115200;

void fillOptions(ComboBox *combo, const QVariantList &options) {
    const QVariant previous = combo->currentData();
    combo->blockSignals(true);
    combo->clear();
    for (const QVariant &v : options) {
        const QVariantMap m = v.toMap();
        combo->addItem(m.value(QStringLiteral("label")).toString(), m.value(QStringLiteral("value")));
    }
    if (previous.isValid()) combo->setCurrentIndex(qMax(0, combo->findData(previous)));
    combo->blockSignals(false);
}

int intData(const ComboBox *combo) { return combo->currentData().toInt(); }

}  // namespace

SerialSettingsPanel::SerialSettingsPanel(AppController *controller, QWidget *parent)
    : QScrollArea(parent), controller_(controller), options_(new SerialOptions(this)) {
    setFrameShape(QFrame::NoFrame);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->setAutoFillBackground(false);
    QLight::ScrollBar::overlayIn(this);

    auto *content = new QWidget(this);
    setWidget(content);
    content->setAutoFillBackground(false);  // setWidget() turns it on

    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(16, 14, 16, 16);
    col->setSpacing(6);

    // --- port --------------------------------------------------------------
    portHeading_ = new ThemedLabel(ThemedLabel::Primary, content);
    portHeading_->setEmphasis(true);
    col->addWidget(portHeading_);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(6);
    grid->setColumnMinimumWidth(0, 72);
    grid->setColumnStretch(1, 1);

    auto makeLabel = [content] { return new ThemedLabel(ThemedLabel::Primary, content); };
    portLabel_ = makeLabel();
    baudLabel_ = makeLabel();
    dataBitsLabel_ = makeLabel();
    parityLabel_ = makeLabel();
    stopBitsLabel_ = makeLabel();
    flowLabel_ = makeLabel();

    portCombo_ = new ComboBox(content);
    portCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    portCombo_->setMinimumContentsLength(6);
    refreshButton_ = new QLight::ToolButton(QLight::FluentIcon(QLight::Icon::ArrowSync), content);
    auto *portRow = new QHBoxLayout;
    portRow->setSpacing(6);
    portRow->addWidget(portCombo_, 1);
    portRow->addWidget(refreshButton_);

    baudCombo_ = new ComboBox(content);
    for (const QVariant &rate : options_->baudRateOptions()) baudCombo_->addItem(rate.toString(), rate);
    baudCombo_->setCurrentIndex(qMax(0, baudCombo_->findData(kDefaultBaud)));
    dataBitsCombo_ = new ComboBox(content);
    parityCombo_ = new ComboBox(content);
    stopBitsCombo_ = new ComboBox(content);
    flowCombo_ = new ComboBox(content);
    for (ComboBox *c : {baudCombo_, dataBitsCombo_, parityCombo_, stopBitsCombo_, flowCombo_}) {
        c->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        c->setMinimumContentsLength(6);
    }

    grid->addWidget(portLabel_, 0, 0);
    grid->addLayout(portRow, 0, 1);
    grid->addWidget(baudLabel_, 1, 0);
    grid->addWidget(baudCombo_, 1, 1);
    grid->addWidget(dataBitsLabel_, 2, 0);
    grid->addWidget(dataBitsCombo_, 2, 1);
    grid->addWidget(parityLabel_, 3, 0);
    grid->addWidget(parityCombo_, 3, 1);
    grid->addWidget(stopBitsLabel_, 4, 0);
    grid->addWidget(stopBitsCombo_, 4, 1);
    grid->addWidget(flowLabel_, 5, 0);
    grid->addWidget(flowCombo_, 5, 1);
    col->addLayout(grid);

    openButton_ = new PortToggleButton(content);
    col->addSpacing(4);
    col->addWidget(openButton_);

    // --- receive -----------------------------------------------------------
    col->addSpacing(12);
    receiveHeading_ = new ThemedLabel(ThemedLabel::Primary, content);
    receiveHeading_->setEmphasis(true);
    col->addWidget(receiveHeading_);
    receiveMode_ = new SegmentedControl(content);
    receiveMode_->setOptions({QStringLiteral("ASCII"), QStringLiteral("HEX")});
    receiveMode_->setSegmentWidth(66);
    col->addWidget(receiveMode_, 0, Qt::AlignLeft);
    timestampCheck_ = new QLight::CheckBox(content);
    wrapCheck_ = new QLight::CheckBox(content);
    echoCheck_ = new QLight::CheckBox(content);
    col->addSpacing(4);
    col->addWidget(timestampCheck_);
    col->addWidget(wrapCheck_);
    col->addWidget(echoCheck_);

    // --- transmit ----------------------------------------------------------
    col->addSpacing(12);
    transmitHeading_ = new ThemedLabel(ThemedLabel::Primary, content);
    transmitHeading_->setEmphasis(true);
    col->addWidget(transmitHeading_);
    transmitMode_ = new SegmentedControl(content);
    transmitMode_->setOptions({QStringLiteral("ASCII"), QStringLiteral("HEX")});
    transmitMode_->setSegmentWidth(66);
    col->addWidget(transmitMode_, 0, Qt::AlignLeft);
    crcCheck_ = new QLight::CheckBox(content);
    col->addSpacing(4);
    col->addWidget(crcCheck_);
    auto *repeatRow = new QHBoxLayout;
    repeatRow->setSpacing(8);
    repeatCheck_ = new QLight::CheckBox(content);
    repeatSpin_ = new QLight::SpinBox(content);
    repeatSpin_->setRange(0, 3600000);
    repeatSpin_->setSingleStep(50);
    repeatSpin_->setMinimumWidth(110);
    msLabel_ = new ThemedLabel(ThemedLabel::Secondary, content);
    repeatRow->addWidget(repeatCheck_);
    repeatRow->addStretch(1);
    repeatRow->addWidget(repeatSpin_);
    repeatRow->addWidget(msLabel_);
    col->addLayout(repeatRow);
    col->addStretch(1);

    // compact rows, so the whole panel fits the default window height
    for (QWidget *w : std::initializer_list<QWidget *>{timestampCheck_, wrapCheck_, echoCheck_, crcCheck_, repeatCheck_})
        w->setFixedHeight(28);

    // --- state -> widgets --------------------------------------------------
    LogListModel *log = controller_->logModel();
    receiveMode_->setCurrentIndex(log->hexMode() ? 1 : 0);
    timestampCheck_->setChecked(log->showTimestamp());
    wrapCheck_->setChecked(true);
    echoCheck_->setChecked(controller_->echoTx());
    transmitMode_->setCurrentIndex(controller_->sendAsHex() ? 1 : 0);
    crcCheck_->setChecked(controller_->crcEnabled());
    repeatCheck_->setChecked(controller_->repeatSendEnabled());
    repeatSpin_->setValue(controller_->repeatIntervalMs());

    connect(log, &LogListModel::hexModeChanged, this,
            [this, log] { receiveMode_->setCurrentIndex(log->hexMode() ? 1 : 0); });
    connect(log, &LogListModel::showTimestampChanged, this,
            [this, log] { timestampCheck_->setChecked(log->showTimestamp()); });
    connect(controller_, &AppController::echoTxChanged, this, [this] { echoCheck_->setChecked(controller_->echoTx()); });
    connect(controller_, &AppController::sendAsHexChanged, this,
            [this] { transmitMode_->setCurrentIndex(controller_->sendAsHex() ? 1 : 0); });
    connect(controller_, &AppController::crcEnabledChanged, this,
            [this] { crcCheck_->setChecked(controller_->crcEnabled()); });
    connect(controller_, &AppController::repeatSendChanged, this, [this] {
        repeatCheck_->setChecked(controller_->repeatSendEnabled());
        if (repeatSpin_->value() != controller_->repeatIntervalMs()) repeatSpin_->setValue(controller_->repeatIntervalMs());
    });

    // --- widgets -> state --------------------------------------------------
    connect(receiveMode_, &SegmentedControl::activated, this, [log](int i) { log->setHexMode(i == 1); });
    connect(timestampCheck_, &QCheckBox::toggled, this, [log](bool on) { log->setShowTimestamp(on); });
    connect(wrapCheck_, &QCheckBox::toggled, this, &SerialSettingsPanel::wrapLinesChanged);
    connect(echoCheck_, &QCheckBox::toggled, controller_, &AppController::setEchoTx);
    connect(transmitMode_, &SegmentedControl::activated, this, [this](int i) { controller_->setSendAsHex(i == 1); });
    connect(crcCheck_, &QCheckBox::toggled, controller_, &AppController::setCrcEnabled);
    connect(repeatCheck_, &QCheckBox::toggled, controller_, &AppController::setRepeatSendEnabled);
    connect(repeatSpin_, &QSpinBox::valueChanged, controller_, &AppController::setRepeatIntervalMs);

    // --- ports -------------------------------------------------------------
    PortListModel *ports = controller_->portListModel();
    connect(ports, &QAbstractItemModel::modelReset, this, &SerialSettingsPanel::syncPortCombo);
    connect(ports, &QAbstractItemModel::rowsInserted, this, &SerialSettingsPanel::syncPortCombo);
    connect(ports, &QAbstractItemModel::rowsRemoved, this, &SerialSettingsPanel::syncPortCombo);
    connect(ports, &QAbstractItemModel::dataChanged, this, &SerialSettingsPanel::syncPortCombo);
    connect(controller_, &AppController::selectedPortNameChanged, this, &SerialSettingsPanel::syncPortCombo);
    connect(portCombo_, &QComboBox::activated, this, [this](int index) {
        if (syncingPorts_) return;
        controller_->setSelectedPortName(controller_->portListModel()->portNameAt(index));
    });
    connect(refreshButton_, &QToolButton::clicked, controller_, &AppController::refreshPorts);

    connect(openButton_, &QAbstractButton::clicked, this, &SerialSettingsPanel::togglePort);
    connect(controller_, &AppController::connectionChanged, this, &SerialSettingsPanel::updateConnectionState);
    connect(controller_, &AppController::currentModelChanged, this, &SerialSettingsPanel::applyModelSerialDefaults);
    connect(controller_, &AppController::currentLanguageChanged, this, &SerialSettingsPanel::rebuildOptionCombos);

    rebuildOptionCombos();
    // QML defaults: 8 data bits, no parity, 1 stop bit, no flow control
    dataBitsCombo_->setCurrentIndex(qMax(0, dataBitsCombo_->findData(8)));
    applyModelSerialDefaults();
    syncPortCombo();
    retranslateUi();
    updateConnectionState();
}

bool SerialSettingsPanel::wrapLines() const { return wrapCheck_->isChecked(); }

void SerialSettingsPanel::changeEvent(QEvent *e) {
    QScrollArea::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) retranslateUi();
}

void SerialSettingsPanel::retranslateUi() {
    portHeading_->setText(tr("Port settings"));
    portLabel_->setText(tr("Port"));
    baudLabel_->setText(tr("Baud rate"));
    dataBitsLabel_->setText(tr("Data bits"));
    parityLabel_->setText(tr("Parity"));
    stopBitsLabel_->setText(tr("Stop bits"));
    flowLabel_->setText(tr("Flow control"));
    portCombo_->setPlaceholderText(tr("No ports found"));
    refreshButton_->setToolTip(tr("Refresh ports"));
    receiveHeading_->setText(tr("Receive"));
    timestampCheck_->setText(tr("Show timestamp"));
    wrapCheck_->setText(tr("Wrap lines"));
    echoCheck_->setText(tr("Echo sent data"));
    transmitHeading_->setText(tr("Transmit"));
    crcCheck_->setText(tr("Append CRC (CRC16/MODBUS)"));
    repeatCheck_->setText(tr("Repeat send"));
    msLabel_->setText(tr("ms"));
    updateConnectionState();
}

void SerialSettingsPanel::rebuildOptionCombos() {
    fillOptions(dataBitsCombo_, options_->dataBitsOptions());
    fillOptions(parityCombo_, options_->parityOptions());
    fillOptions(stopBitsCombo_, options_->stopBitsOptions());
    fillOptions(flowCombo_, options_->flowControlOptions());
}

void SerialSettingsPanel::syncPortCombo() {
    PortListModel *ports = controller_->portListModel();
    syncingPorts_ = true;
    portCombo_->blockSignals(true);
    portCombo_->clear();
    for (int row = 0; row < ports->rowCount(); ++row) {
        const QModelIndex idx = ports->index(row);
        const QString name = idx.data(PortListModel::PortNameRole).toString();
        const QString description = idx.data(PortListModel::DescriptionRole).toString();
        const QString chip = idx.data(PortListModel::ChipLabelRole).toString();
        QString label = name;
        if (!chip.isEmpty()) label += QStringLiteral("  ·  ") + chip;
        portCombo_->addItem(label, name);
        portCombo_->setItemData(row, description.isEmpty() ? tr("Unknown device") : description, Qt::ToolTipRole);
    }
    portCombo_->setCurrentIndex(ports->indexOfPortName(controller_->selectedPortName()));
    portCombo_->setToolTip(portCombo_->currentIndex() >= 0
                               ? portCombo_->itemData(portCombo_->currentIndex(), Qt::ToolTipRole).toString()
                               : QString());
    portCombo_->blockSignals(false);
    syncingPorts_ = false;
}

void SerialSettingsPanel::applyModelSerialDefaults() {
    const QVariantMap d = controller_->serialDefaultsForModel(controller_->currentModelId());
    if (!d.value(QStringLiteral("present")).toBool()) return;
    const auto select = [](ComboBox *combo, const QVariant &value) {
        const int i = combo->findData(value.toInt());
        if (i >= 0) combo->setCurrentIndex(i);
    };
    select(baudCombo_, d.value(QStringLiteral("baudRate")));
    select(dataBitsCombo_, d.value(QStringLiteral("dataBits")));
    select(parityCombo_, d.value(QStringLiteral("parity")));
    select(stopBitsCombo_, d.value(QStringLiteral("stopBits")));
    select(flowCombo_, d.value(QStringLiteral("flowControl")));
}

void SerialSettingsPanel::updateConnectionState() {
    const bool open = controller_->portOpen();
    for (QWidget *w : std::initializer_list<QWidget *>{portCombo_, refreshButton_, baudCombo_, dataBitsCombo_,
                                                       parityCombo_, stopBitsCombo_, flowCombo_}) {
        w->setEnabled(!open);
    }
    openButton_->setText(open ? tr("Close port") : tr("Open port"));
    openButton_->setDanger(open);
}

void SerialSettingsPanel::togglePort() {
    if (controller_->portOpen()) {
        controller_->closePort();
        return;
    }
    const QString port = portCombo_->currentIndex() >= 0 ? portCombo_->currentData().toString() : QString();
    controller_->openPort(port, baudCombo_->currentData().toInt(), intData(dataBitsCombo_), intData(parityCombo_),
                          intData(stopBitsCombo_), intData(flowCombo_));
}

}  // namespace ui
