#pragma once

#include <QPoint>
#include <QString>

#include "QLight/Export.h"

class QWidget;

namespace QLight {

// Fluent-style tooltip: rounded, shadowed flyout with a short fade-in.
namespace ToolTip {

enum class Placement {
    AtCursor, // below-right of `pos`, like QToolTip
    Above,    // centered horizontally on `pos`, sitting above it
};

// Routes every QEvent::ToolTip of a widget with a non-empty toolTip() through
// this implementation instead of QToolTip. Call once after creating the
// QApplication; repeated calls are harmless. Tooltips that widgets produce
// themselves (e.g. per-item tooltips in views) keep using QToolTip, which is
// re-themed to match.
QLIGHT_EXPORT void installGlobally();

// Shows (or updates) the tooltip. `owner` is the widget it belongs to; the
// tooltip hides when the owner sees a leave/press/key/wheel event. Plain text
// and rich text are both accepted.
QLIGHT_EXPORT void showText(const QPoint &globalPos, const QString &text, QWidget *owner = nullptr,
                            Placement placement = Placement::AtCursor);

QLIGHT_EXPORT void hideText();
QLIGHT_EXPORT bool isVisible();

} // namespace ToolTip
} // namespace QLight
