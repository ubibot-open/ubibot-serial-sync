// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "ui/app_dialog.h"

class AppController;

namespace QLight {
class LineEdit;
class ProgressBar;
class PushButton;
class SpinBox;
}  // namespace QLight

namespace ui {

class ComboBox;
class LibraryUpdateDialog;
class SegmentedControl;
class ThemedLabel;

// Tools -> Settings: language, fonts, theme, command library version/update,
// version info and "Restore defaults". Every change applies immediately.
class SettingsDialog : public AppDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(AppController *controller, QWidget *parent = nullptr);
    void openDialog();

protected:
    void retranslateUi() override;

private:
    void syncFromController();
    void updateLibraryState();

    AppController *controller_;
    bool syncing_ = false;

    ThemedLabel *languageLabel_;
    ComboBox *languageCombo_;
    ThemedLabel *systemFontLabel_;
    ComboBox *systemFontCombo_;
    ThemedLabel *systemSizeLabel_;
    QLight::SpinBox *systemSizeSpin_;
    ThemedLabel *logFontLabel_;
    ComboBox *logFontCombo_;
    ThemedLabel *logSizeLabel_;
    QLight::SpinBox *logSizeSpin_;
    ThemedLabel *themeLabel_;
    SegmentedControl *themeControl_;
    ThemedLabel *libraryLabel_;
    QLight::LineEdit *libraryVersion_;
    ThemedLabel *libraryBadge_;
    QLight::PushButton *checkButton_;
    ThemedLabel *versionLabel_;
    ThemedLabel *version_;
    ThemedLabel *platformLabel_;
    ThemedLabel *platform_;
    ThemedLabel *supportLabel_;
    ThemedLabel *support_;
    ThemedLabel *devicesLabel_;
    ThemedLabel *devices_;
    QLight::PushButton *restoreButton_;
    QLight::PushButton *closeButton_;
    LibraryUpdateDialog *updateDialog_ = nullptr;
};

// Result of "Check for updates" for the command library, with
// "Download and apply" when a newer library is available.
class LibraryUpdateDialog : public AppDialog {
    Q_OBJECT
public:
    explicit LibraryUpdateDialog(AppController *controller, QWidget *parent = nullptr);

protected:
    void retranslateUi() override;

private:
    void updateState();

    AppController *controller_;
    ThemedLabel *message_;
    QLight::ProgressBar *busy_;
    QLight::PushButton *downloadButton_;
    QLight::PushButton *okButton_;
};

// Help -> Check for software update (entry point currently disabled, see MainWindow).
class SoftwareUpdateDialog : public AppDialog {
    Q_OBJECT
public:
    explicit SoftwareUpdateDialog(AppController *controller, QWidget *parent = nullptr);

protected:
    void retranslateUi() override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    void updateState();

    AppController *controller_;
    ThemedLabel *headline_;
    ThemedLabel *forcedNote_;
    ThemedLabel *message_;
    QLight::ProgressBar *progress_;
    ThemedLabel *percent_;
    QLight::PushButton *laterButton_;
    QLight::PushButton *cancelButton_;
    QLight::PushButton *updateButton_;
    QLight::PushButton *retryButton_;
    QLight::PushButton *okButton_;
};

}  // namespace ui
