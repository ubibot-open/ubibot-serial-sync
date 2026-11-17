// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <QAbstractButton>
#include <QColor>
#include <QFont>
#include <QFrame>
#include <QLabel>
#include <QStringList>
#include <QWidget>

#include <functional>

#include <QLight/ComboBox.h>

// Shared look-and-feel for the Widgets UI. The controls themselves come from
// the vendored QLight library (third_party/qlight); this file adds the app's
// own colours on top of QLight's palette plus the handful of small widgets the
// v4 design needs that QLight doesn't ship (flat cards, segmented control,
// status pill, the red/blue "Open port" button).
namespace ui {

// UbiBot brand colour, used as QLight's accent.
inline const QColor kAccent(0x00, 0x68, 0xB7);

// Colours that are not part of QLight::Palette.
struct AppColors {
    QColor accentText;   // accent used for text (lighter in dark mode)
    QColor accentTint;   // selected-row / running-row fill
    QColor danger;       // close-port button, error text
    QColor dangerHover;
    QColor success;      // "connected" dot
    QColor idle;         // "not connected" dot
    QColor logTx;
    QColor logRx;
    QColor logSys;
    QColor logErr;
};

const AppColors &colors();

// Applies "light" / "dark" to QLight and refreshes the application QPalette so
// plain Qt widgets (QLabel, QScrollArea, ...) follow the theme too.
void applyTheme(const QString &mode);

// Monospace font for data (log view, send box, command text).
QFont monoFont(int pixelSize = 13, const QString &family = {});

// Adds a one-shot connection that calls `fn` now and on every theme change.
void onThemeChanged(QObject *context, const std::function<void()> &fn);

// Rounded, hairline-bordered surface in the card colour (no shadow): the
// v4 design's left panel, data monitor, send box, ...
class Panel : public QFrame {
    Q_OBJECT
public:
    explicit Panel(QWidget *parent = nullptr);
    // When set, the panel paints the window background colour instead of the card colour.
    void setRecessed(bool on);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    bool recessed_ = false;
};

// Plain label whose colour follows the theme.
class ThemedLabel : public QLabel {
    Q_OBJECT
public:
    enum Role { Primary, Secondary, Accent, Danger, Success };
    explicit ThemedLabel(Role role = Primary, QWidget *parent = nullptr);
    ThemedLabel(const QString &text, Role role, QWidget *parent = nullptr);
    void setRole(Role role);
    // Convenience: semibold (section headings) and/or a pixel size delta from the app font.
    void setEmphasis(bool semibold, int sizeDelta = 0);

protected:
    void changeEvent(QEvent *e) override;

private:
    void apply();
    Role role_;
    bool semibold_ = false;
    int sizeDelta_ = 0;
};

// One-pixel hairline in the card stroke colour.
class Divider : public QWidget {
    Q_OBJECT
public:
    explicit Divider(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};

// Two or more mutually exclusive options drawn as one bordered strip, the
// selected one filled with the accent colour (ASCII/HEX, Light/Dark, ...).
class SegmentedControl : public QWidget {
    Q_OBJECT
public:
    explicit SegmentedControl(QWidget *parent = nullptr);
    void setOptions(const QStringList &labels);
    void setOptionText(int index, const QString &text);
    int currentIndex() const { return current_; }
    void setCurrentIndex(int index);
    // Fixed width per segment (0 = size to text).
    void setSegmentWidth(int width);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }

signals:
    // Only emitted for user clicks / key presses, not for setCurrentIndex().
    void activated(int index);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    QRectF segmentRect(int i) const;
    int segmentAt(const QPoint &pos) const;

    QStringList labels_;
    int current_ = 0;
    int hover_ = -1;
    int segmentWidth_ = 0;
};

// "● Connected · COM5" pill in the command bar, and the plain dot + text in
// the status bar (setFramed(false)).
class StatusPill : public QWidget {
    Q_OBJECT
public:
    explicit StatusPill(QWidget *parent = nullptr);
    void setState(bool connected, const QString &text);
    void setFramed(bool framed);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    bool connected_ = false;
    bool framed_ = true;
    QString text_;
};

// The large "Open port" / "Close port" button: accent when closed, red when open.
class PortToggleButton : public QAbstractButton {
    Q_OBJECT
public:
    explicit PortToggleButton(QWidget *parent = nullptr);
    void setDanger(bool danger);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    bool danger_ = false;
};

// Equal-width text tabs with an accent underline under the selected one
// ("Serial" / "Device commands" at the top of the left panel).
class PivotBar : public QWidget {
    Q_OBJECT
public:
    explicit PivotBar(QWidget *parent = nullptr);
    void setTabs(const QStringList &labels);
    void setTabText(int index, const QString &text);
    int currentIndex() const { return current_; }
    void setCurrentIndex(int index);
    QSize sizeHint() const override;

signals:
    void currentChanged(int index);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    int tabAt(const QPoint &pos) const;
    QStringList labels_;
    int current_ = 0;
    int hover_ = -1;
};

// QLight combo box that simply does nothing when clicked while empty (instead
// of opening an empty flyout), e.g. the port list with no device plugged in.
class ComboBox : public QLight::ComboBox {
    Q_OBJECT
public:
    using QLight::ComboBox::ComboBox;
    void showPopup() override;
};

// Opens `url` in the system browser.
void openUrl(const QString &url);

// Human-readable byte count ("214 B", "1.2 KB", ...).
QString formatBytes(qint64 n);

}  // namespace ui
