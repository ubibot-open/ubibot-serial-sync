#pragma once

#include <QColor>
#include <QFont>
#include <QObject>

#include "QLight/Export.h"

namespace QLight {

enum class ThemeMode { Light, Dark, Auto };

// Design tokens resolved for the current theme. Semi-transparent fills are
// intentional: they let controls blend with whatever surface they sit on.
struct Palette {
    // surfaces
    QColor windowBg;
    QColor cardBg;
    QColor cardStroke;

    // text
    QColor textPrimary;
    QColor textSecondary;
    QColor textDisabled;
    QColor textPlaceholder;
    QColor textOnAccent;
    QColor textOnAccentDisabled;

    // standard control
    QColor controlFill;
    QColor controlFillHover;
    QColor controlFillPressed;
    QColor controlFillDisabled;
    QColor controlStrokeTop;
    QColor controlStrokeBottom;
    QColor controlStrokeStrong; // checkbox/switch outline, input bottom edge
    QColor inputFillFocused;

    // subtle (transparent) control
    QColor subtleFillHover;
    QColor subtleFillPressed;

    // accent control
    QColor accent;
    QColor accentHover;
    QColor accentPressed;
    QColor accentDisabled;
    QColor accentStrokeTop;
    QColor accentStrokeBottom;

    // slider / progress
    QColor trackFill;
    QColor thumbFill;
    QColor thumbStroke;

    // status
    QColor success;
    QColor caution;
    QColor critical;
    QColor statusInfoBg;     // InfoBar backgrounds
    QColor statusSuccessBg;
    QColor statusCautionBg;
    QColor statusCriticalBg;

    QColor dimOverlay;       // smoke layer behind modal dialogs

    QColor focusStroke;
    QColor shadowColor;

    // shape
    qreal radiusControl = 4;
    qreal radiusCard = 8;
};

class QLIGHT_EXPORT ThemeManager : public QObject {
    Q_OBJECT
public:
    // Requires a QGuiApplication instance.
    static ThemeManager &instance();

    ThemeMode mode() const { return m_mode; }
    void setMode(ThemeMode mode);

    // Effective theme after resolving ThemeMode::Auto.
    bool isDark() const { return m_dark; }

    QColor accentColor() const { return m_accentBase; }
    void setAccentColor(const QColor &color);

    const Palette &palette() const { return m_palette; }

    // Sensible cross-platform UI font (14px, Segoe UI / YaHei / PingFang / Noto).
    static QFont defaultFont();

signals:
    void themeChanged();

private:
    ThemeManager();
    void rebuild(bool emitSignal);

    ThemeMode m_mode = ThemeMode::Auto;
    QColor m_accentBase{0x00, 0x78, 0xD4};
    bool m_dark = false;
    Palette m_palette;
};

} // namespace QLight
