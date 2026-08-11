#pragma once

#include "core/device_library.h"

#include <QWizard>

// Three-step "connect a new device" flow: pick a detected serial port, pick
// the UbiBot model it is, confirm & finish. On accept(), MainWindow reads
// selectedPort()/selectedModelId() and opens the port at 115200 8-N-1 with
// that model's command set loaded.
class ConnectionWizard : public QWizard {
    Q_OBJECT
public:
    explicit ConnectionWizard(const DeviceLibrary *library, QWidget *parent = nullptr);

    QString selectedPort() const;
    QString selectedModelId() const;

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
};
