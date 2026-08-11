#pragma once

#include "core/device_library.h"

#include <QDialog>

class QRadioButton;
class QLabel;
class QPushButton;

// "Settings & About" dialog: interface language switch plus static
// version/library/support info. This is where AppLanguage actually gets
// changed at runtime (see LanguageManager::setLanguage()).
class SettingsAboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsAboutDialog(const DeviceLibrary *library, QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void retranslateUi();
    void checkForUpdate();

    const DeviceLibrary *library_;

    class QGroupBox *languageGroup_;
    QRadioButton *bilingualRadio_;
    QRadioButton *chineseRadio_;
    QRadioButton *englishRadio_;

    class QGroupBox *libraryGroup_;
    QLabel *libraryVersionLabel_;
    QLabel *libraryStatusTag_;
    QPushButton *checkUpdateButton_;

    QLabel *versionCaption_;
    QLabel *versionValue_;
    QLabel *platformCaption_;
    QLabel *platformValue_;
    QLabel *supportCaption_;
    QLabel *supportValue_;
    QLabel *devicesCaption_;
    QLabel *devicesValue_;

    class QDialogButtonBox *buttonBox_;
};
