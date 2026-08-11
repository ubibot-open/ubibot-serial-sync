#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;

// Left-hand panel shown in "Remote support" mode.
//
// NOTE: this is a UI placeholder. The design calls for a support agent to
// enter a code and connect directly to this app, which requires either a
// relay/signaling server or a peer-to-peer transport -- neither exists yet.
// The panel generates a plausible-looking session code and OTP and lets the
// user set the intended permissions, but "start session" surfaces an
// explicit "not implemented yet" notice rather than pretending to connect
// anyone. Wire this up to a real transport in a follow-up.
class RemoteAssistPanel : public QWidget {
    Q_OBJECT
public:
    explicit RemoteAssistPanel(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void retranslateUi();
    void regenerateCode();

    QLabel *titleLabel_;
    QLabel *introLabel_;
    QLabel *codeCaptionLabel_;
    QLabel *codeLabel_;
    QLabel *codeHintLabel_;
    QPushButton *copyButton_;
    QPushButton *regenerateButton_;
    QLabel *otpCaptionLabel_;
    QLineEdit *otpEdit_;
    QCheckBox *allowPortControlCheck_;
    QCheckBox *shareLogCheck_;
    QCheckBox *fullDesktopCheck_;
    QPushButton *sessionButton_;
    QLabel *statusLabel_;

    QString code_;
    QString otp_;
};
