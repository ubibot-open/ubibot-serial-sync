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
    standardButtons: Dialog.Close
    // Dialogs don't reliably inherit ApplicationWindow's own palette --
    // see Theme.qml. This is the one place the user actually picks
    // light/dark, so it especially can't be left stuck on the wrong colors.
    // `palette` alone re-themes the controls inside but Fusion's default
    // Dialog background apparently doesn't rebind to `palette.window`
    // live, so the canvas itself needs an explicit override too.
    palette: Theme.palette
    background: Rectangle { color: Theme.background; border.color: Theme.divider; border.width: 1 }
    // Same rebind gap as `background` above, but for the title strip --
    // Fusion's default Dialog header is its own separately-drawn piece.
    header: Label {
        text: root.title
        font.bold: true
        padding: 12
        color: Theme.text
        background: Rectangle { color: Theme.background }
    }

    // Keeps languageCombo/fontFamilyCombo in sync when the underlying
    // AppController properties change from outside a user pick on those
    // combos themselves -- namely restoreDefaults() below. Both combos only
    // set their currentIndex imperatively (Component.onCompleted/onActivated)
    // rather than via a live binding, since an editable ComboBox's
    // currentIndex is otherwise clobbered by the user's own typing; that
    // means a change originating on the C++ side needs an explicit nudge to
    // show up here. fontSizeSpin needs no equivalent handler -- its `value`
    // is a genuine declarative binding to AppController.logFontSize, so it
    // already re-reads on change.
    Connections {
        target: AppController
        function onCurrentLanguageChanged() {
            languageCombo.currentIndex = languageCombo.indexOfValue(AppController.currentLanguage)
        }
        function onLogFontChanged() {
            fontFamilyCombo.currentIndex = fontFamilyCombo.find(AppController.logFontFamily)
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

    contentItem: ColumnLayout {
        spacing: 14

        GroupBox {
            title: qsTr("Interface language")
            Layout.fillWidth: true

            ComboBox {
                id: languageCombo
                anchors.fill: parent
                textRole: "nativeName"
                valueRole: "code"
                model: AppController.availableLanguages()
                Component.onCompleted: currentIndex = indexOfValue(AppController.currentLanguage)
                onActivated: AppController.currentLanguage = currentValue
            }
        }

        GroupBox {
            title: qsTr("Data monitor font")
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 10

                ComboBox {
                    id: fontFamilyCombo
                    Layout.fillWidth: true
                    editable: true
                    model: AppController.availableFontFamilies()
                    Component.onCompleted: currentIndex = find(AppController.logFontFamily)
                    onActivated: AppController.logFontFamily = currentText
                    onAccepted: AppController.logFontFamily = editText
                }

                Label { text: qsTr("Size") }

                SpinBox {
                    id: fontSizeSpin
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

            RowLayout {
                anchors.fill: parent
                ButtonGroup { id: themeGroup }
                RadioButton {
                    id: lightThemeRadio
                    text: qsTr("Light")
                    checked: AppController.themeMode !== "dark"
                    ButtonGroup.group: themeGroup
                    onClicked: AppController.themeMode = "light"
                }
                RadioButton {
                    id: darkThemeRadio
                    text: qsTr("Dark")
                    checked: AppController.themeMode === "dark"
                    ButtonGroup.group: themeGroup
                    onClicked: AppController.themeMode = "dark"
                }
            }
        }

        GroupBox {
            title: qsTr("Command library")
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                Label { text: AppController.libraryVersion; font.family: Theme.monoFont }
                Label {
                    text: qsTr("Up to date")
                    color: Theme.accent800
                    padding: 4
                    background: Rectangle { color: Theme.accentTint }
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Check for updates")
                    onClicked: updateResultDialog.open()
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 16
            rowSpacing: 4

            Label { text: qsTr("Version"); font.pixelSize: 11; color: Theme.textMuted }
            Label { text: qsTr("Platform"); font.pixelSize: 11; color: Theme.textMuted }
            Label { text: "1.0.0 (Qt 6.11)" }
            Label { text: "Windows · macOS · Linux" }

            Label { text: qsTr("Support"); font.pixelSize: 11; color: Theme.textMuted }
            Label { text: qsTr("Devices"); font.pixelSize: 11; color: Theme.textMuted }
            Label { text: "support@ubibot.com" }
            Label { text: qsTr("%1 models · %2 commands").arg(AppController.modelCount).arg(AppController.commandCount) }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4

            Button {
                text: qsTr("Restore defaults")
                onClicked: AppController.restoreDefaultSettings()
            }
            Item { Layout.fillWidth: true }
        }
    }

    Dialog {
        id: updateResultDialog
        title: qsTr("Command library")
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok
        palette: Theme.palette
        background: Rectangle { color: Theme.background; border.color: Theme.divider; border.width: 1 }
        header: Label {
            text: updateResultDialog.title
            font.bold: true
            padding: 12
            color: Theme.text
            background: Rectangle { color: Theme.background }
        }
        Label {
            width: 320
            wrapMode: Text.WordWrap
            text: AppController.checkForLibraryUpdate()
            color: Theme.text
        }
    }
}
