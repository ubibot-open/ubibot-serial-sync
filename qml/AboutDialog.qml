import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UbiBot

// "About" dialog: static, read-only app info (version, build time,
// description, company/website, GitHub link, Qt licensing notice) --
// opened from the Help menu. Kept separate from SettingsAboutDialog.qml
// (language/font/theme/library-update *settings*, all user-configurable)
// since nothing here is something the user changes.
Dialog {
    id: root
    title: qsTr("About")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 440
    // Dialogs don't reliably inherit ApplicationWindow's own palette/font --
    // see Theme.qml's file-level comment -- so both need the explicit
    // assignment here too, same as every other standalone Dialog/Popup.
    palette: Theme.palette
    font.family: Theme.baseFontFamily
    font.pixelSize: Theme.baseFontSize
    background: DialogCard {}
    // See ModalDim.qml -- keeps this modal's dimming out of the frameless
    // main window's shadow-margin ring.
    Overlay.modal: ModalDim {}
    header: Label {
        text: root.title
        font.bold: true
        padding: 16
        color: Theme.text
    }
    footer: DialogButtonBox {
        palette: Theme.palette
        Button {
            text: qsTr("Close")
            palette: Theme.palette
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

    Pane {
        anchors.fill: parent
        padding: 16
        contentItem: ColumnLayout {
            spacing: 14

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Image {
                    source: "qrc:/icons/app.png"
                    sourceSize: Qt.size(40, 40)
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                }
                ColumnLayout {
                    spacing: 2
                    Label {
                        text: qsTr("UbiBot Serial Assistant")
                        font.bold: true
                        font.pixelSize: Theme.baseFontSize + 2
                    }
                    Label {
                        text: qsTr("Version %1").arg(AppController.appVersion)
                        color: Theme.textMuted
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: Theme.textMuted
                text: qsTr("A serial-port debugging tool for UbiBot IoT devices (WS1, WS1 Pro, GS1-AL4G1RS, SP1, and more), built with Qt 6, QML, and C++17.")
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.divider }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 16
                rowSpacing: 8

                Label { text: qsTr("Build time"); font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
                Label { text: AppController.buildTime; font.family: Theme.monoFont }

                Label { text: qsTr("Company"); font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
                Label { text: qsTr("UbiBot · United States of America") }

                Label { text: qsTr("Website"); font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
                Label {
                    textFormat: Text.RichText
                    linkColor: Theme.accent
                    text: "<a href=\"https://www.ubibot.com\">https://www.ubibot.com</a>"
                    onLinkActivated: (link) => Qt.openUrlExternally(link)
                }

                Label { text: qsTr("GitHub"); font.pixelSize: Theme.baseFontSize - 1; color: Theme.textMuted }
                Label {
                    textFormat: Text.RichText
                    linkColor: Theme.accent
                    text: "<a href=\"https://github.com/ubibot-open/ubibot-serial-sync\">github.com/ubibot-open/ubibot-serial-sync</a>"
                    onLinkActivated: (link) => Qt.openUrlExternally(link)
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.divider }

            // See README.md's "License"/"Qt 6 module licensing" sections for
            // the full compatibility check this summarizes -- every Qt
            // module this app actually links against ships under LGPLv3
            // (same as this project itself), and the release process
            // (windeployqt) links Qt dynamically rather than statically, so
            // end users can substitute a compatible Qt build per LGPLv3
            // §4(d) without this app needing a Qt commercial license.
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.baseFontSize - 2
                color: Theme.textMuted
                text: qsTr("Built with Qt %1 under the GNU Lesser General Public License v3 (LGPLv3). Qt is a trademark of The Qt Company Ltd. Qt is linked dynamically, so the bundled Qt libraries may be replaced with a compatible LGPLv3 build.").arg(AppController.qtVersion)
            }
        }
    }
}
