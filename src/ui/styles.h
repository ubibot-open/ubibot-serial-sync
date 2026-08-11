#pragma once

#include "core/log_entry.h"

#include <QColor>
#include <QString>

// Colors and the app stylesheet, both lifted from the original design's
// design-token palette (light theme). Kept as free functions/constants
// rather than a class since nothing here needs to be instantiated.
namespace Styles {

constexpr auto kBackground = "#f2f2f3";
constexpr auto kSurface = "#e9e9ea";
constexpr auto kText = "#1d1f20";
constexpr auto kAccent = "#5980a6";
constexpr auto kAccent700 = "#416180";
constexpr auto kAccent800 = "#2c455d";
constexpr auto kDivider = "#c9c9ca";
constexpr auto kError = "#aa3333";
constexpr auto kMonoFont = "Consolas, 'Cascadia Mono', 'JetBrains Mono', monospace";

QString appStyleSheet();
QColor colorForLogKind(LogKind kind);

}  // namespace Styles
