// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "ui/app_dialog.h"

class AppController;
class QGridLayout;
class QLabel;
class QVBoxLayout;

namespace QLight {
class CheckBox;
class LineEdit;
class PushButton;
}  // namespace QLight

namespace ui {

class SegmentedControl;
class ThemedLabel;

// Save the session log (text / CSV / hex dump), optionally keep logging to disk.
class SaveLogDialog : public AppDialog {
    Q_OBJECT
public:
    explicit SaveLogDialog(AppController *controller, QWidget *parent = nullptr);
    void openDialog();

protected:
    void retranslateUi() override;

private:
    void save();

    AppController *controller_;
    ThemedLabel *fileLabel_;
    ThemedLabel *locationLabel_;
    ThemedLabel *formatLabel_;
    QLight::LineEdit *fileField_;
    QLight::LineEdit *locationField_;
    QLight::PushButton *browseButton_;
    SegmentedControl *format_;
    QLight::CheckBox *autoRotate_;
    ThemedLabel *errorLabel_;
    QLight::PushButton *cancelButton_;
    QLight::PushButton *saveButton_;
};

class AboutDialog : public AppDialog {
    Q_OBJECT
public:
    explicit AboutDialog(AppController *controller, QWidget *parent = nullptr);

protected:
    void retranslateUi() override;

private:
    AppController *controller_;
    ThemedLabel *name_;
    ThemedLabel *version_;
    ThemedLabel *description_;
    ThemedLabel *buildTimeLabel_;
    ThemedLabel *buildTime_;
    ThemedLabel *companyLabel_;
    ThemedLabel *company_;
    ThemedLabel *websiteLabel_;
    QLabel *website_;
    ThemedLabel *githubLabel_;
    QLabel *github_;
    ThemedLabel *license_;
    QLight::PushButton *closeButton_;
};

// Shown from the "driver not installed" bar: explains the problem.
class DriverHintDialog : public AppDialog {
    Q_OBJECT
public:
    explicit DriverHintDialog(AppController *controller, QWidget *parent = nullptr);

signals:
    void downloadRequested();

protected:
    void retranslateUi() override;

private:
    AppController *controller_;
    ThemedLabel *text_;
    QLight::CheckBox *dontShow_;
    QLight::PushButton *downloadButton_;
    QLight::PushButton *closeButton_;
};

// Help -> Driver downloads: every known driver with its install state and a
// download link.
class DriverListDialog : public AppDialog {
    Q_OBJECT
public:
    explicit DriverListDialog(AppController *controller, QWidget *parent = nullptr);
    void openDialog();

protected:
    void retranslateUi() override;

private:
    void rebuild();

    AppController *controller_;
    QVBoxLayout *rows_;
    QLight::PushButton *closeButton_;
};

}  // namespace ui
