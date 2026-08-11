#pragma once

#include "core/log_manager.h"

#include <QDialog>

class QLineEdit;
class QRadioButton;
class QCheckBox;

// "Save session log" dialog: exports everything currently in the data
// monitor to a file immediately, and optionally turns on continuous,
// daily-rotating logging for the rest of the session.
class SaveLogDialog : public QDialog {
    Q_OBJECT
public:
    SaveLogDialog(const QString &suggestedBaseName, const QString &initialDirectory,
                  QWidget *parent = nullptr);

    QString directory() const;
    QString baseFileName() const;  // without extension
    QString filePath() const;      // directory + baseFileName + extension for the chosen format
    LogFileFormat format() const;
    bool autoRotateDaily() const;

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi(const QString &suggestedBaseName, const QString &initialDirectory);
    void retranslateUi();
    void browseForDirectory();
    QString extensionForFormat() const;

    QLineEdit *fileNameEdit_;
    QLineEdit *locationEdit_;
    class QPushButton *browseButton_;
    class QGroupBox *formatGroup_;
    QRadioButton *plainRadio_;
    QRadioButton *csvRadio_;
    QRadioButton *hexRadio_;
    QCheckBox *autoRotateCheck_;
    class QLabel *fileNameLabel_;
    class QLabel *locationLabel_;
    class QDialogButtonBox *buttonBox_;
};
