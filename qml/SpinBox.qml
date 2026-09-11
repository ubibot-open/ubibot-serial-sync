// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick.Controls.Fusion as Fusion

// Shadows QtQuick.Controls' own SpinBox app-wide -- see ComboBox.qml's
// comment for how/why. Same reasoning as TextField/ComboBox: Fusion's own
// 4px padding reads short.
Fusion.SpinBox {
    padding: 6
}
