// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/ui_common.h"

#include <QApplication>
#include <QDesktopServices>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QUrl>

#include <cmath>

#include <QLight/Theme.h>

namespace ui {

namespace {

AppColors g_colors;

// One device pixel, but never thinner than that at fractional scale factors.
qreal hairline(const QWidget *w) {
    const qreal dpr = w->devicePixelRatioF();
    return qMax<qreal>(1.0, std::floor(dpr)) / dpr;
}

void rebuildColors() {
    const bool dark = QLight::ThemeManager::instance().isDark();
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    AppColors c;
    if (dark) {
        c.accentText = QColor(0x4D, 0xA3, 0xE8);
        c.accentTint = QColor(0x1F, 0x3A, 0x52);
        c.danger = QColor(0xC4, 0x2B, 0x1C);
        c.dangerHover = QColor(0xD6, 0x3A, 0x2B);
        c.success = QColor(0x6C, 0xCB, 0x5F);
        c.idle = QColor(0x8A, 0x8A, 0x8A);
        c.logTx = QColor(0x4D, 0xA3, 0xE8);
        c.logRx = p.textPrimary;
        c.logSys = QColor(0x9D, 0x9D, 0x9D);
        c.logErr = QColor(0xFF, 0x99, 0xA4);
    } else {
        c.accentText = kAccent;
        c.accentTint = QColor(0xE5, 0xF1, 0xFA);
        c.danger = QColor(0xC4, 0x2B, 0x1C);
        c.dangerHover = QColor(0xA8, 0x24, 0x17);
        c.success = QColor(0x0F, 0x7B, 0x0F);
        c.idle = QColor(0xA1, 0x9F, 0x9D);
        c.logTx = kAccent;
        c.logRx = p.textPrimary;
        c.logSys = QColor(0x60, 0x5E, 0x5C);
        c.logErr = QColor(0xC4, 0x2B, 0x1C);
    }
    g_colors = c;
}

void applyAppPalette() {
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    QPalette pal = QApplication::palette();
    for (auto group : {QPalette::Active, QPalette::Inactive}) {
        pal.setColor(group, QPalette::Window, p.windowBg);
        pal.setColor(group, QPalette::WindowText, p.textPrimary);
        pal.setColor(group, QPalette::Base, p.cardBg);
        pal.setColor(group, QPalette::AlternateBase, p.windowBg);
        pal.setColor(group, QPalette::Text, p.textPrimary);
        pal.setColor(group, QPalette::Button, p.cardBg);
        pal.setColor(group, QPalette::ButtonText, p.textPrimary);
        pal.setColor(group, QPalette::Highlight, p.accent);
        pal.setColor(group, QPalette::HighlightedText, p.textOnAccent);
        pal.setColor(group, QPalette::PlaceholderText, p.textPlaceholder);
        pal.setColor(group, QPalette::Link, g_colors.accentText);
        pal.setColor(group, QPalette::ToolTipBase, p.cardBg);
        pal.setColor(group, QPalette::ToolTipText, p.textPrimary);
    }
    pal.setColor(QPalette::Disabled, QPalette::WindowText, p.textDisabled);
    pal.setColor(QPalette::Disabled, QPalette::Text, p.textDisabled);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, p.textDisabled);
    pal.setColor(QPalette::Disabled, QPalette::Window, p.windowBg);
    pal.setColor(QPalette::Disabled, QPalette::Base, p.cardBg);
    QApplication::setPalette(pal);
}

}  // namespace

const AppColors &colors() {
    if (!g_colors.danger.isValid()) rebuildColors();
    return g_colors;
}

void applyTheme(const QString &mode) {
    auto &theme = QLight::ThemeManager::instance();
    theme.setAccentColor(kAccent);
    // rebuild before the mode change notifies widgets, so every themeChanged
    // handler already sees the new app colours
    static bool hooked = false;
    if (!hooked) {
        hooked = true;
        QObject::connect(&theme, &QLight::ThemeManager::themeChanged, qApp, [] {
            rebuildColors();
            applyAppPalette();
        });
    }
    const QLight::ThemeMode target = mode == QLatin1String("dark") ? QLight::ThemeMode::Dark : QLight::ThemeMode::Light;
    if (theme.mode() != target) {
        theme.setMode(target);  // emits themeChanged -> rebuildColors / applyAppPalette
    }
    rebuildColors();
    applyAppPalette();
}

QFont monoFont(int pixelSize, const QString &family) {
    QFont f;
    QStringList families;
    if (!family.isEmpty()) families << family;
    families << QStringLiteral("Cascadia Mono") << QStringLiteral("Consolas") << QStringLiteral("Menlo")
             << QStringLiteral("DejaVu Sans Mono") << QStringLiteral("Microsoft YaHei UI")
             << QStringLiteral("PingFang SC") << QStringLiteral("Noto Sans CJK SC") << QStringLiteral("monospace");
    f.setFamilies(families);
    f.setStyleHint(QFont::Monospace);
    f.setPixelSize(pixelSize);
    return f;
}

void onThemeChanged(QObject *context, const std::function<void()> &fn) {
    QObject::connect(&QLight::ThemeManager::instance(), &QLight::ThemeManager::themeChanged, context,
                     [fn] { fn(); });
    fn();
}

// --- Panel ---------------------------------------------------------------

Panel::Panel(QWidget *parent) : QFrame(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
    onThemeChanged(this, [this] { update(); });
}

void Panel::setRecessed(bool on) {
    recessed_ = on;
    update();
}

void Panel::paintEvent(QPaintEvent *) {
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal bw = hairline(this);
    const QRectF r = QRectF(rect()).adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2);
    painter.setPen(QPen(p.cardStroke, bw));
    painter.setBrush(recessed_ ? p.windowBg : p.cardBg);
    painter.drawRoundedRect(r, p.radiusCard, p.radiusCard);
}

// --- ThemedLabel ---------------------------------------------------------

ThemedLabel::ThemedLabel(Role role, QWidget *parent) : QLabel(parent), role_(role) {
    onThemeChanged(this, [this] { apply(); });
}

ThemedLabel::ThemedLabel(const QString &text, Role role, QWidget *parent) : ThemedLabel(role, parent) {
    setText(text);
}

void ThemedLabel::setRole(Role role) {
    role_ = role;
    apply();
}

void ThemedLabel::setEmphasis(bool semibold, int sizeDelta) {
    semibold_ = semibold;
    sizeDelta_ = sizeDelta;
    apply();
}

void ThemedLabel::changeEvent(QEvent *e) {
    QLabel::changeEvent(e);
    // the app font can change at runtime (Settings -> System font); labels
    // with their own emphasis font must re-derive it from the new one
    if (e->type() == QEvent::ApplicationFontChange) apply();
}

void ThemedLabel::apply() {
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    const AppColors &c = colors();
    QColor color;
    switch (role_) {
    case Primary: color = p.textPrimary; break;
    case Secondary: color = p.textSecondary; break;
    case Accent: color = c.accentText; break;
    case Danger: color = c.logErr; break;
    case Success: color = c.success; break;
    }
    QPalette pal = palette();
    pal.setColor(QPalette::WindowText, color);
    pal.setColor(QPalette::Text, color);
    setPalette(pal);

    if (semibold_ || sizeDelta_ != 0) {
        QFont f = QApplication::font();
        if (sizeDelta_ != 0 && f.pixelSize() > 0) f.setPixelSize(f.pixelSize() + sizeDelta_);
        else if (sizeDelta_ != 0) f.setPointSizeF(f.pointSizeF() + sizeDelta_ * 0.75);
        if (semibold_) f.setWeight(QFont::DemiBold);
        setFont(f);
    }
}

// --- Divider -------------------------------------------------------------

Divider::Divider(Qt::Orientation orientation, QWidget *parent) : QWidget(parent) {
    if (orientation == Qt::Horizontal) setFixedHeight(1);
    else setFixedWidth(1);
    onThemeChanged(this, [this] { update(); });
}

void Divider::paintEvent(QPaintEvent *) {
    QPainter(this).fillRect(rect(), QLight::ThemeManager::instance().palette().cardStroke);
}

// --- SegmentedControl ----------------------------------------------------

SegmentedControl::SegmentedControl(QWidget *parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    onThemeChanged(this, [this] { update(); });
}

void SegmentedControl::setOptions(const QStringList &labels) {
    labels_ = labels;
    current_ = qBound(0, current_, qMax(0, labels_.size() - 1));
    updateGeometry();
    update();
}

void SegmentedControl::setOptionText(int index, const QString &text) {
    if (index < 0 || index >= labels_.size()) return;
    labels_[index] = text;
    updateGeometry();
    update();
}

void SegmentedControl::setCurrentIndex(int index) {
    if (index < 0 || index >= labels_.size() || index == current_) return;
    current_ = index;
    update();
}

void SegmentedControl::setSegmentWidth(int width) {
    segmentWidth_ = width;
    updateGeometry();
}

QSize SegmentedControl::sizeHint() const {
    const QFontMetrics fm(font());
    int w = 0;
    for (const QString &l : labels_) {
        w += segmentWidth_ > 0 ? qMax(segmentWidth_, fm.horizontalAdvance(l) + 24) : fm.horizontalAdvance(l) + 40;
    }
    return QSize(w + 6, 32);
}

QRectF SegmentedControl::segmentRect(int i) const {
    const QFontMetrics fm(font());
    qreal x = 3;
    for (int k = 0; k < labels_.size(); ++k) {
        const qreal w = segmentWidth_ > 0 ? qMax(segmentWidth_, fm.horizontalAdvance(labels_[k]) + 24)
                                          : fm.horizontalAdvance(labels_[k]) + 40;
        if (k == i) {
            // stretch the last segment to the right edge when the widget is wider than its hint
            if (k == labels_.size() - 1) return QRectF(x, 3, width() - 3 - x, height() - 6);
            return QRectF(x, 3, w, height() - 6);
        }
        x += w;
    }
    return {};
}

int SegmentedControl::segmentAt(const QPoint &pos) const {
    for (int i = 0; i < labels_.size(); ++i)
        if (segmentRect(i).contains(pos)) return i;
    return -1;
}

void SegmentedControl::paintEvent(QPaintEvent *) {
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal bw = hairline(this);
    const QRectF outer = QRectF(rect()).adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2);
    painter.setPen(QPen(isEnabled() ? p.controlStrokeBottom : p.controlStrokeTop, bw));
    painter.setBrush(p.controlFill);
    painter.drawRoundedRect(outer, p.radiusControl, p.radiusControl);

    for (int i = 0; i < labels_.size(); ++i) {
        const QRectF r = segmentRect(i);
        QColor text = isEnabled() ? p.textPrimary : p.textDisabled;
        if (i == current_) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(isEnabled() ? p.accent : p.accentDisabled);
            painter.drawRoundedRect(r, 3, 3);
            text = p.textOnAccent;
        } else if (i == hover_ && isEnabled()) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(p.subtleFillHover);
            painter.drawRoundedRect(r, 3, 3);
        }
        painter.setPen(text);
        painter.drawText(r, Qt::AlignCenter, labels_[i]);
    }
    if (hasFocus() && focusPolicy() != Qt::NoFocus && property("keyboardFocus").toBool()) {
        painter.setPen(QPen(p.focusStroke, 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), p.radiusControl, p.radiusControl);
    }
}

void SegmentedControl::mousePressEvent(QMouseEvent *e) {
    setProperty("keyboardFocus", false);
    const int i = segmentAt(e->position().toPoint());
    if (i >= 0 && i != current_) {
        current_ = i;
        update();
        emit activated(i);
    }
}

void SegmentedControl::mouseMoveEvent(QMouseEvent *e) {
    const int i = segmentAt(e->position().toPoint());
    if (i != hover_) {
        hover_ = i;
        update();
    }
}

void SegmentedControl::leaveEvent(QEvent *) {
    hover_ = -1;
    update();
}

void SegmentedControl::keyPressEvent(QKeyEvent *e) {
    int next = current_;
    if (e->key() == Qt::Key_Left) next = current_ - 1;
    else if (e->key() == Qt::Key_Right) next = current_ + 1;
    else {
        QWidget::keyPressEvent(e);
        return;
    }
    setProperty("keyboardFocus", true);
    if (next >= 0 && next < labels_.size()) {
        current_ = next;
        update();
        emit activated(next);
    }
}

// --- StatusPill ----------------------------------------------------------

StatusPill::StatusPill(QWidget *parent) : QWidget(parent) {
    onThemeChanged(this, [this] { update(); });
}

void StatusPill::setState(bool connected, const QString &text) {
    connected_ = connected;
    text_ = text;
    updateGeometry();
    update();
}

void StatusPill::setFramed(bool framed) {
    framed_ = framed;
    updateGeometry();
    update();
}

QSize StatusPill::sizeHint() const {
    const QFontMetrics fm(font());
    const int w = fm.horizontalAdvance(text_) + (framed_ ? 44 : 20);
    return QSize(framed_ ? qMax(w, 160) : w, framed_ ? 28 : fm.height() + 4);
}

void StatusPill::paintEvent(QPaintEvent *) {
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal bw = hairline(this);
    int x = 0;
    if (framed_) {
        const QRectF r = QRectF(rect()).adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2);
        painter.setPen(QPen(p.cardStroke, bw));
        painter.setBrush(p.cardBg);
        painter.drawRoundedRect(r, r.height() / 2, r.height() / 2);
        x = 14;
    } else {
        x = 2;
    }
    const qreal cy = height() / 2.0;
    painter.setPen(Qt::NoPen);
    painter.setBrush(connected_ ? colors().success : colors().idle);
    painter.drawEllipse(QPointF(x + 4, cy), 4, 4);
    painter.setPen(framed_ ? p.textPrimary : p.textSecondary);
    painter.drawText(QRect(x + 16, 0, width() - x - 16, height()), Qt::AlignVCenter | Qt::AlignLeft, text_);
}

// --- PortToggleButton ----------------------------------------------------

PortToggleButton::PortToggleButton(QWidget *parent) : QAbstractButton(parent) {
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAttribute(Qt::WA_Hover);
    onThemeChanged(this, [this] { update(); });
}

void PortToggleButton::setDanger(bool danger) {
    danger_ = danger;
    update();
}

QSize PortToggleButton::sizeHint() const { return QSize(QFontMetrics(font()).horizontalAdvance(text()) + 48, 40); }

void PortToggleButton::enterEvent(QEnterEvent *e) {
    QAbstractButton::enterEvent(e);
    update();
}

void PortToggleButton::leaveEvent(QEvent *e) {
    QAbstractButton::leaveEvent(e);
    update();
}

void PortToggleButton::paintEvent(QPaintEvent *) {
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    const AppColors &c = colors();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor fill;
    if (!isEnabled()) fill = p.accentDisabled;
    else if (danger_) fill = (isDown() || underMouse()) ? c.dangerHover : c.danger;
    else fill = isDown() ? p.accentPressed : underMouse() ? p.accentHover : p.accent;
    painter.setPen(Qt::NoPen);
    painter.setBrush(fill);
    painter.drawRoundedRect(QRectF(rect()), p.radiusControl, p.radiusControl);
    painter.setPen(danger_ ? QColor(Qt::white) : p.textOnAccent);
    painter.drawText(rect(), Qt::AlignCenter, text());
    if (hasFocus() && property("keyboardFocus").toBool()) {
        painter.setPen(QPen(p.focusStroke, 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), p.radiusControl, p.radiusControl);
    }
}

// --- PivotBar ------------------------------------------------------------

PivotBar::PivotBar(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    onThemeChanged(this, [this] { update(); });
}

void PivotBar::setTabs(const QStringList &labels) {
    labels_ = labels;
    update();
}

void PivotBar::setTabText(int index, const QString &text) {
    if (index < 0 || index >= labels_.size()) return;
    labels_[index] = text;
    update();
}

void PivotBar::setCurrentIndex(int index) {
    if (index < 0 || index >= labels_.size() || index == current_) return;
    current_ = index;
    update();
    emit currentChanged(index);
}

QSize PivotBar::sizeHint() const { return QSize(200, 44); }

int PivotBar::tabAt(const QPoint &pos) const {
    if (labels_.isEmpty() || !rect().contains(pos)) return -1;
    return qBound(0, pos.x() * int(labels_.size()) / qMax(1, width()), int(labels_.size()) - 1);
}

void PivotBar::paintEvent(QPaintEvent *) {
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const int n = int(labels_.size());
    if (n == 0) return;
    const qreal w = qreal(width()) / n;
    for (int i = 0; i < n; ++i) {
        const QRectF r(i * w, 0, w, height());
        QFont f = font();
        if (i == current_) f.setWeight(QFont::DemiBold);
        painter.setFont(f);
        painter.setPen(i == current_ ? colors().accentText : (i == hover_ ? p.textPrimary : p.textSecondary));
        painter.drawText(r, Qt::AlignCenter, labels_[i]);
        if (i == current_) {
            const qreal lineW = qMin<qreal>(QFontMetricsF(f).horizontalAdvance(labels_[i]) + 4, w - 16);
            painter.setPen(Qt::NoPen);
            painter.setBrush(p.accent);
            painter.drawRoundedRect(QRectF(r.center().x() - lineW / 2, height() - 4, lineW, 3), 1.5, 1.5);
        }
    }
}

void PivotBar::mousePressEvent(QMouseEvent *e) {
    const int i = tabAt(e->position().toPoint());
    if (i >= 0) setCurrentIndex(i);
}

void PivotBar::mouseMoveEvent(QMouseEvent *e) {
    const int i = tabAt(e->position().toPoint());
    if (i != hover_) {
        hover_ = i;
        update();
    }
}

void PivotBar::leaveEvent(QEvent *) {
    hover_ = -1;
    update();
}

void ComboBox::showPopup() {
    if (count() == 0) return;
    QLight::ComboBox::showPopup();
}

void openUrl(const QString &url) { QDesktopServices::openUrl(QUrl(url)); }

QString formatBytes(qint64 n) {
    if (n < 1024) return QStringLiteral("%1 B").arg(n);
    if (n < 1024 * 1024) return QStringLiteral("%1 KB").arg(n / 1024.0, 0, 'f', 1);
    if (n < 1024LL * 1024 * 1024) return QStringLiteral("%1 MB").arg(n / (1024.0 * 1024.0), 0, 'f', 1);
    return QStringLiteral("%1 GB").arg(n / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
}

}  // namespace ui
