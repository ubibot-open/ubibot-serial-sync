#pragma once

#include "core/device_library.h"

#include <QDialog>

class QComboBox;
class QLabel;
class QPushButton;

// "Settings & About" dialog: interface language switch plus static
// version/library/support info. This is where the app language actually
// gets changed at runtime (see LanguageManager::setLanguage()).
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
    // A dropdown rather than radio buttons: this list is meant to grow past
    // a dozen languages, which a row (or even a column) of radio buttons
    // can't hold. Each item's display text is that language's own native
    // name (see LanguageInfo), so it doesn't need retranslating.
    QComboBox *languageCombo_;

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
