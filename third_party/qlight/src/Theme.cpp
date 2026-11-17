#include "QLight/Theme.h"

#include "Accessibility.h"

#include <QGuiApplication>
#include <QStyleHints>

namespace QLight {

namespace {

QColor mix(const QColor &a, const QColor &b, qreal t) // t of a, (1-t) of b
{
    return QColor::fromRgbF(a.redF() * t + b.redF() * (1 - t),
                            a.greenF() * t + b.greenF() * (1 - t),
                            a.blueF() * t + b.blueF() * (1 - t));
}

qreal luminance(const QColor &c)
{
    return 0.2126 * c.redF() + 0.7152 * c.greenF() + 0.0722 * c.blueF();
}

Palette buildPalette(bool dark, const QColor &accentBase)
{
    Palette p;

    QColor accent = accentBase;
    // UbiBot patch: upstream lifts the accent's lightness to 0.62 in dark mode, which turns
    // the brand blue (#0068B7) into a pale blue that needs black text. The app's dark design
    // keeps the brand colour for filled controls (white text) and uses a lighter blue only
    // for accent-coloured text, which it handles itself.
    const QColor base = dark ? QColor(0x20, 0x20, 0x20) : QColor(0xF3, 0xF3, 0xF3);
    p.accent = accent;
    p.accentHover = mix(accent, base, 0.9);
    p.accentPressed = mix(accent, base, 0.8);
    p.textOnAccent = luminance(accent) > 0.6 ? QColor(0, 0, 0) : QColor(255, 255, 255);

    if (dark) {
        p.windowBg = QColor(0x20, 0x20, 0x20);
        p.cardBg = QColor(0x2B, 0x2B, 0x2B);
        p.cardStroke = QColor(0x3A, 0x3A, 0x3A);

        p.textPrimary = QColor(255, 255, 255);
        p.textSecondary = QColor(0xC5, 0xC5, 0xC5);
        p.textDisabled = QColor(255, 255, 255, 92);
        p.textOnAccentDisabled = QColor(255, 255, 255, 135);

        p.controlFill = QColor(255, 255, 255, 15);
        p.controlFillHover = QColor(255, 255, 255, 21);
        p.controlFillPressed = QColor(255, 255, 255, 8);
        p.controlFillDisabled = QColor(255, 255, 255, 11);
        p.controlStrokeTop = QColor(255, 255, 255, 24);
        p.controlStrokeBottom = QColor(255, 255, 255, 12);
        p.controlStrokeStrong = QColor(255, 255, 255, 138);
        p.inputFillFocused = QColor(0x1F, 0x1F, 0x1F);
        p.textPlaceholder = QColor(255, 255, 255, 140);

        p.subtleFillHover = QColor(255, 255, 255, 15);
        p.subtleFillPressed = QColor(255, 255, 255, 10);

        p.accentDisabled = QColor(255, 255, 255, 40);

        p.trackFill = QColor(255, 255, 255, 138);
        p.thumbFill = QColor(0x45, 0x45, 0x45);
        p.thumbStroke = QColor(0, 0, 0, 70);
        p.success = QColor(0x6C, 0xCB, 0x5F);
        p.caution = QColor(0xFC, 0xE1, 0x00);
        p.critical = QColor(0xFF, 0x99, 0xA4);
        p.statusInfoBg = QColor(0x2F, 0x2F, 0x2F);
        p.statusSuccessBg = QColor(0x39, 0x3D, 0x1B);
        p.statusCautionBg = QColor(0x43, 0x35, 0x19);
        p.statusCriticalBg = QColor(0x44, 0x27, 0x26);
        p.dimOverlay = QColor(0, 0, 0, 120);
    } else {
        p.windowBg = QColor(0xF3, 0xF3, 0xF3);
        p.cardBg = QColor(0xFF, 0xFF, 0xFF);
        p.cardStroke = QColor(0xE5, 0xE5, 0xE5);

        p.textPrimary = QColor(0x1A, 0x1A, 0x1A);
        p.textSecondary = QColor(0x5C, 0x5C, 0x5C);
        p.textDisabled = QColor(0, 0, 0, 92);
        p.textOnAccentDisabled = QColor(255, 255, 255);

        p.controlFill = QColor(255, 255, 255, 179);
        p.controlFillHover = QColor(249, 249, 249, 128);
        p.controlFillPressed = QColor(249, 249, 249, 77);
        p.controlFillDisabled = QColor(249, 249, 249, 77);
        p.controlStrokeTop = QColor(0, 0, 0, 15);
        p.controlStrokeBottom = QColor(0, 0, 0, 41);
        p.controlStrokeStrong = QColor(0, 0, 0, 155);
        p.inputFillFocused = QColor(255, 255, 255);
        p.textPlaceholder = QColor(0, 0, 0, 155);

        p.subtleFillHover = QColor(0, 0, 0, 15);
        p.subtleFillPressed = QColor(0, 0, 0, 10);

        p.accentDisabled = QColor(0, 0, 0, 55);

        p.trackFill = QColor(0, 0, 0, 112);
        p.thumbFill = QColor(255, 255, 255);
        p.thumbStroke = QColor(0, 0, 0, 26);
        p.success = QColor(0x0F, 0x7B, 0x0F);
        p.caution = QColor(0x9D, 0x5D, 0x00);
        p.critical = QColor(0xC4, 0x2B, 0x1C);
        p.statusInfoBg = QColor(0xF6, 0xF6, 0xF6);
        p.statusSuccessBg = QColor(0xDF, 0xF6, 0xDD);
        p.statusCautionBg = QColor(0xFF, 0xF4, 0xCE);
        p.statusCriticalBg = QColor(0xFD, 0xE7, 0xE9);
        p.dimOverlay = QColor(0, 0, 0, 77);
    }
    p.accentStrokeTop = QColor(255, 255, 255, 20);
    p.accentStrokeBottom = QColor(0, 0, 0, 60);
    p.focusStroke = p.textPrimary;
    p.shadowColor = dark ? QColor(0, 0, 0, 90) : QColor(0, 0, 0, 36);
    return p;
}

} // namespace

ThemeManager &ThemeManager::instance()
{
    static ThemeManager mgr;
    return mgr;
}

ThemeManager::ThemeManager()
{
    detail::installAccessibility();
    if (auto *hints = QGuiApplication::styleHints()) {
        connect(hints, &QStyleHints::colorSchemeChanged, this, [this] {
            if (m_mode == ThemeMode::Auto)
                rebuild(true);
        });
    }
    rebuild(false);
}

void ThemeManager::setMode(ThemeMode mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    rebuild(true);
}

void ThemeManager::setAccentColor(const QColor &color)
{
    if (!color.isValid() || m_accentBase == color)
        return;
    m_accentBase = color;
    rebuild(true);
}

void ThemeManager::rebuild(bool emitSignal)
{
    switch (m_mode) {
    case ThemeMode::Light: m_dark = false; break;
    case ThemeMode::Dark: m_dark = true; break;
    case ThemeMode::Auto:
        m_dark = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
        break;
    }
    m_palette = buildPalette(m_dark, m_accentBase);
    if (emitSignal)
        emit themeChanged();
}

QFont ThemeManager::defaultFont()
{
    QFont f;
    f.setFamilies({"Segoe UI Variable Text", "Segoe UI", "Microsoft YaHei UI", "PingFang SC",
                   "Noto Sans CJK SC", "Noto Sans"});
    f.setPixelSize(14);
    f.setHintingPreference(QFont::PreferNoHinting);
    return f;
}

} // namespace QLight
