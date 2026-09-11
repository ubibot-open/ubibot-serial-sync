// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

import QtQuick
import QtQuick.Controls.Fusion as Fusion
import UbiBot

Fusion.Menu {
    id: control
    Binding { target: control.background; property: "border.color"; value: Theme.dialogBorder }
}
