#pragma once

#include "core/device_library.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QGridLayout;
class QPushButton;

// The parameter-entry strip that appears above the send box when the user
// picks a command from CommandLibraryPanel that needs arguments (e.g.
// "AT+INTERVAL=<sec>"). Shows one field per parameter plus a live preview of
// the literal string that will be sent.
class CommandParamsPanel : public QWidget {
    Q_OBJECT
public:
    explicit CommandParamsPanel(QWidget *parent = nullptr);

    void setCommand(const DeviceCommand &cmd);

signals:
    void sendRequested(const QString &resolvedCommand);
    void cancelled();

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void retranslateUi();
    void updatePreview();

    DeviceCommand command_;
    QHash<QString, QString> values_;

    QLabel *nameLabel_;
    QLabel *tagLabel_;
    QPushButton *cancelButton_;
    QGridLayout *fieldsLayout_;
    QLabel *previewLabel_;
    QPushButton *sendButton_;
    QVector<QLineEdit *> fieldEdits_;
};
