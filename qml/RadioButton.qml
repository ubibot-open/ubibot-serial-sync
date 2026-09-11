// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Controls.Fusion as Fusion

// Shadows QtQuick.Controls' own RadioButton app-wide -- see ComboBox.qml's
// comment for how/why, and CheckBox.qml's for why the indicator needs a
// Binding rather than a plain property to resize.
Fusion.RadioButton {
    id: control
    padding: 6

    Binding { target: control.indicator; property: "implicitWidth"; value: 18 }
    Binding { target: control.indicator; property: "implicitHeight"; value: 18 }
}
