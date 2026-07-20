import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3
import ADBStudio.Presentation
import "components" as Components
import "pages" as Pages

ApplicationWindow {
    id: window
    required property ThemeService themeService
    required property DeviceDashboardViewModel dashboardViewModel
    required property string applicationVersion
    width: 1180
    height: 760
    minimumWidth: 720
    minimumHeight: 520
    visible: true
    title: qsTr("ADB Studio")
    font.family: window.themeService.fontFamily

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            Image {
                source: "qrc:/assets/tray-adb.png"
                sourceSize.height: 30
                fillMode: Image.PreserveAspectFit
                Accessible.name: qsTr("ADB Studio")
            }
            Item { Layout.fillWidth: true }
            TextField {
                Layout.preferredWidth: 260
                placeholderText: qsTr("Search")
                Accessible.name: qsTr("Search ADB Studio")
            }
            ComboBox {
                model: [qsTr("System theme"), qsTr("Light theme"), qsTr("Dark theme")]
                currentIndex: window.themeService.themeMode
                Accessible.name: qsTr("Application theme")
                onActivated: window.themeService.setThemeMode(currentIndex)
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Pane {
            Layout.fillHeight: true
            Layout.preferredWidth: window.width < 900 ? 76 : 220
            ColumnLayout {
                anchors.fill: parent
                spacing: 4
                ItemDelegate { text: window.width < 900 ? qsTr("Home") : qsTr("Device dashboard"); highlighted: true; Layout.fillWidth: true; Accessible.description: qsTr("Open device dashboard") }
                ItemDelegate { text: window.width < 900 ? qsTr("Mirror") : qsTr("Screen mirroring"); enabled: false; Layout.fillWidth: true }
                ItemDelegate { text: window.width < 900 ? qsTr("Files") : qsTr("File manager"); enabled: false; Layout.fillWidth: true }
                ItemDelegate { text: window.width < 900 ? qsTr("Tools") : qsTr("Developer tools"); enabled: false; Layout.fillWidth: true }
                Item { Layout.fillHeight: true }
                ItemDelegate { text: qsTr("Settings"); enabled: false; Layout.fillWidth: true }
            }
        }

        Pages.DashboardPage {
            themeService: window.themeService
            dashboardViewModel: window.dashboardViewModel
            deviceImageSource: "qrc:/assets/logo-adb.png"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 24
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            Components.StatusBadge {
                text: window.dashboardViewModel.state
                statusColor: window.dashboardViewModel.connected ? window.themeService.success : window.themeService.warning
            }
            Item { Layout.fillWidth: true }
            Label { text: qsTr("Version %1").arg(window.applicationVersion) }
        }
    }
}
