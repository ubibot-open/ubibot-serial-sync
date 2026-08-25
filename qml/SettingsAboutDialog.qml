import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// "Settings & About" dialog: interface language switch plus static
// version/library/support info. The language list comes from
// AppController.availableLanguages() -- a dropdown rather than radio
// buttons, since it's meant to grow past a dozen entries.
Dialog {
    id: root
    title: qsTr("Settings & About")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 460
    // Dialogs don't reliably inherit ApplicationWindow's own palette --
    // see Theme.qml. This is the one place the user actually picks
    // light/dark, so it especially can't be left stuck on the wrong colors.
    // `palette` alone re-themes the controls inside but Fusion's default
    // Dialog background apparently doesn't rebind to `palette.window`
    // live, so the canvas itself needs an explicit override too.
    palette: Theme.palette
    font.family: Theme.baseFontFamily
    font.pixelSize: Theme.baseFontSize
    // See DialogCard.qml for why this dialog needs its own elevated
    // surface + border + shadow instead of a plain Theme.background fill.
    background: DialogCard {}
    // Same rebind gap as `background` above, but for the title strip --
    // Fusion's default Dialog header is its own separately-drawn piece.
    // Uses Theme.surface (not Theme.background) to match DialogCard's fill
    // above so the header reads as part of the same panel, not a seam.
    header: Label {
        text: root.title
        font.bold: true
        padding: 16
        color: Theme.text
        // background: Rectangle { color: Theme.surface }
    }
    // A Popup's children apparently don't reliably pick up a *live* change
    // to an inherited palette either (only their own direct assignment
    // reacts) -- see Theme.qml. Was `standardButtons: Dialog.Close`, but
    // that auto-generated button can't be reached to give it its own
    // `palette:`, so it's spelled out explicitly here instead (identical
    // behavior: a single Close button that dismisses the dialog).
    footer: DialogButtonBox {
        palette: Theme.palette
        // Theme.surface again, matching DialogCard/header above.
        // background: Rectangle { color: Theme.surface }
        Button {
            text: qsTr("Close")
            palette: Theme.palette
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

    // Keeps languageCombo/fontFamilyCombo/systemFontFamilyCombo in sync when
    // the underlying AppController properties change from outside a user
    // pick on those combos themselves -- namely restoreDefaults() below. All
    // three combos only set their currentIndex imperatively
    // (Component.onCompleted/onActivated) rather than via a live binding,
    // since an editable ComboBox's currentIndex is otherwise clobbered by
    // the user's own typing; that means a change originating on the C++
    // side needs an explicit nudge to show up here. fontSizeSpin/
    // systemFontSizeSpin need no equivalent handler -- their `value` is a
    // genuine declarative binding to AppController.logFontSize/
    // systemFontSize, so they already re-read on change.
    Connections {
        target: AppController
        function onCurrentLanguageChanged() {
            languageCombo.currentIndex = languageCombo.indexOfValue(AppController.currentLanguage)
        }
        function onLogFontChanged() {
            fontFamilyCombo.currentIndex = fontFamilyCombo.find(AppController.logFontFamily)
        }
        function onSystemFontChanged() {
            systemFontFamilyCombo.currentIndex = systemFontFamilyCombo.find(AppController.systemFontFamily)
        }
        // RadioButton.checked is reassigned internally the instant it's
        // clicked, same hazard as TextArea.text (see Main.qml's inputField
        // comment) -- an initial declarative `checked: ...` binding gets
        // severed by that assignment, so a later external change (Restore
        // defaults, below) wouldn't otherwise reach the button that was
        // clicked. Re-deriving both explicitly on every themeModeChanged
        // sidesteps that regardless of which binding is still alive.
        function onThemeModeChanged() {
            lightThemeRadio.checked = AppController.themeMode !== "dark"
            darkThemeRadio.checked = AppController.themeMode === "dark"
        }
    }
    Pane {
        anchors.fill: parent
        padding: 16
        contentItem: ColumnLayout {
            spacing: 16

            // Every GroupBox/ComboBox/RadioButton/Button below gets its own
            // explicit `palette: Theme.palette` for the same reason the footer
            // buttons above do -- these are Fusion-styled controls that read
            // `control.palette.xxx` for their own chrome (frame, fill, ...);
            // relying on inheriting it from `root` above doesn't reliably
            // survive a *live* theme switch, only a fresh one at open time.
            GroupBox {
                title: qsTr("Interface language")
                Layout.fillWidth: true
                palette: Theme.palette
                padding: 0

                ComboBox {
                    id: languageCombo
                    anchors.fill: parent
                    palette: Theme.palette
                    textRole: "nativeName"
                    valueRole: "code"
                    model: AppController.availableLanguages()
                    Component.onCompleted: currentIndex = indexOfValue(AppController.currentLanguage)
                    onActivated: AppController.currentLanguage = currentValue
                }
            }

            GroupBox {
                title: qsTr("System font")
                Layout.fillWidth: true
                palette: Theme.palette
                padding: 0

                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    ComboBox {
                        id: systemFontFamilyCombo
                        Layout.fillWidth: true
                        palette: Theme.palette
                        editable: false
                        model: AppController.availableFontFamilies()
                        Component.onCompleted: currentIndex = find(AppController.systemFontFamily)
                        onActivated: AppController.systemFontFamily = currentText
                        onAccepted: AppController.systemFontFamily = editText
                    }

                    Label { text: qsTr("Size") }

                    SpinBox {
                        id: systemFontSizeSpin
                        palette: Theme.palette
                        from: 8
                        to: 32
                        value: AppController.systemFontSize
                        onValueModified: AppController.systemFontSize = value
                    }
                }
            }

            GroupBox {
                title: qsTr("Data monitor font")
                Layout.fillWidth: true
                palette: Theme.palette
                padding: 0

                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    ComboBox {
                        id: fontFamilyCombo
                        Layout.fillWidth: true
                        palette: Theme.palette
                        editable: false
                        model: AppController.availableFontFamilies()
                        Component.onCompleted: currentIndex = find(AppController.logFontFamily)
                        onActivated: AppController.logFontFamily = currentText
                        onAccepted: AppController.logFontFamily = editText
                    }

                    Label { text: qsTr("Size") }

                    SpinBox {
                        id: fontSizeSpin
                        palette: Theme.palette
                        from: 8
                        to: 32
                        value: AppController.logFontSize
                        onValueModified: AppController.logFontSize = value
                    }
                }
            }

            GroupBox {
                title: qsTr("Theme")
                Layout.fillWidth: true
                palette: Theme.palette
                padding: 0

                RowLayout {
                    anchors.fill: parent
                    ButtonGroup { id: themeGroup }
                    RadioButton {
                        id: lightThemeRadio
                        text: qsTr("Light")
                        palette: Theme.palette
                        checked: AppController.themeMode !== "dark"
                        ButtonGroup.group: themeGroup
                        onClicked: AppController.themeMode = "light"
                    }
                    RadioButton {
                        id: darkThemeRadio
                        text: qsTr("Dark")
                        palette: Theme.palette
                        checked: AppController.themeMode === "dark"
                        ButtonGroup.group: themeGroup
                        onClicked: AppController.themeMode = "dark"
                    }
                }
            }

            GroupBox {
                title: qsTr("Command library")
                Layout.fillWidth: true
                palette: Theme.palette
                padding: 0

                RowLayout {
                    anchors.fill: parent
                    Label { text: AppController.libraryVersion; font.family: Theme.monoFont }
                    Label {
                        // "Up to date" only once a check has actually run and
                        // found nothing newer -- before the first check this
                        // just shows nothing rather than a guess.
                        visible: AppController.libraryUpdateState === "upToDate"
                        text: qsTr("Up to date")
                        color: Theme.accent800
                        padding: 2
                        background: Rectangle { color: Theme.accentTint }
                    }
                    Label {
                        visible: AppController.libraryUpdateState === "updateAvailable"
                        text: qsTr("Update available")
                        color: Theme.text
                        padding: 2
                        background: Rectangle { color: Theme.accent }
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: qsTr("Check for updates")
                        palette: Theme.palette
                        enabled: AppController.libraryUpdateState !== "checking" && AppController.libraryUpdateState !== "downloading"
                        onClicked: {
                            AppController.checkForLibraryUpdate();
                            updateResultDialog.open();
                        }
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 16
                rowSpacing: 4

                Label { text: qsTr("Version"); font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
                Label { text: qsTr("Platform"); font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
                Label { text: qsTr("%1 (Qt %2)").arg(AppController.appVersion).arg(AppController.qtVersion) }
                Label { text: "Windows · macOS · Linux" }

                Label { text: qsTr("Support"); font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
                Label { text: qsTr("Devices"); font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
                Label { text: "support@ubibot.com" }
                Label { text: qsTr("%1 models · %2 commands").arg(AppController.modelCount).arg(AppController.commandCount) }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 6

                Button {
                    text: qsTr("Restore defaults")
                    palette: Theme.palette
                    onClicked: AppController.restoreDefaultSettings()
                }
                Item { Layout.fillWidth: true }
            }
        }
}
    Dialog {
        id: updateResultDialog
        title: qsTr("Command library")
        modal: true
        anchors.centerIn: Overlay.overlay
        palette: Theme.palette
        font.family: Theme.baseFontFamily
        font.pixelSize: Theme.baseFontSize
        background: DialogCard {}
        header: Label {
            text: updateResultDialog.title
            font.bold: true
            padding: 14
            color: Theme.text
            // background: Rectangle { color: Theme.surface }
        }
        // Was `standardButtons: Dialog.Ok` -- see root's footer above for why
        // that auto-generated button had to be spelled out explicitly instead.
        footer: DialogButtonBox {
            palette: Theme.palette
            // background: Rectangle { color: Theme.surface }
            Button {
                text: qsTr("Download and apply")
                palette: Theme.palette
                visible: AppController.libraryUpdateAvailable
                enabled: AppController.libraryUpdateState !== "downloading"
                DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
                onClicked: AppController.downloadLibraryUpdate()
            }
            Button {
                text: qsTr("OK")
                palette: Theme.palette
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
        }
        ColumnLayout {
            width: 320
            spacing: 8
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: AppController.libraryUpdateMessage
                color: Theme.text
            }
            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                running: AppController.libraryUpdateState === "checking" || AppController.libraryUpdateState === "downloading"
                visible: running
            }
        }
    }
}
