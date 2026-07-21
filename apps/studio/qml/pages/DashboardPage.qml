import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3
import ADBStudio.Presentation
import "../components" as Components

ScrollView {
    id: page
    required property ThemeService themeService
    required property DeviceDashboardViewModel dashboardViewModel
    required property WorkspaceViewModel workspaceViewModel
    required property url deviceImageSource
    contentWidth: availableWidth
    Accessible.role: Accessible.Pane
    Accessible.name: qsTr("Device dashboard")

    ColumnLayout {
        width: page.availableWidth
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                Label { text: qsTr("Device dashboard"); font.pixelSize: 28; font.weight: Font.DemiBold }
                Label { text: qsTr("Live connection and performance status"); color: palette.placeholderText }
            }
            Components.PrimaryButton {
                text: qsTr("Refresh")
                Accessible.description: qsTr("Scan for connected devices")
                onClicked: page.dashboardViewModel.refresh()
            }
        }

        Components.ConnectionWizard {
            Layout.fillWidth: true
            themeService: page.themeService
            dashboardViewModel: page.dashboardViewModel
        }

        GroupBox {
            title: qsTr("Quick actions")
            Layout.fillWidth: true
            GridLayout {
                anchors.fill: parent
                columns: page.availableWidth >= 800 ? 4 : page.availableWidth >= 460 ? 2 : 1
                Button {
                    text: qsTr("Screen mirroring")
                    onClicked: page.workspaceViewModel.selectModule("mirror")
                }
                Button {
                    text: qsTr("Devices")
                    onClicked: page.workspaceViewModel.selectModule("devices")
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: page.availableWidth >= 820 ? 4 : page.availableWidth >= 420 ? 2 : 1
            columnSpacing: 12
            rowSpacing: 12
            Components.InfoCard {
                Layout.fillWidth: true
                title: qsTr("Device health")
                value: qsTr("%1 / 100").arg(page.dashboardViewModel.healthScore)
            }
            Components.InfoCard {
                Layout.fillWidth: true
                title: qsTr("USB status")
                value: page.dashboardViewModel.usbStatus
            }
            Components.InfoCard {
                Layout.fillWidth: true
                title: qsTr("Wireless status")
                value: page.dashboardViewModel.wirelessStatus
            }
            Components.InfoCard {
                Layout.fillWidth: true
                title: qsTr("CPU / RAM")
                value: page.dashboardViewModel.connected
                       ? qsTr("%1 / %2").arg(page.dashboardViewModel.cpu, page.dashboardViewModel.memory)
                       : qsTr("Not verified")
            }
        }

        Components.DeviceCard {
            Layout.fillWidth: true
            connected: page.dashboardViewModel.connected
            deviceImageSource: page.deviceImageSource
            manufacturer: page.dashboardViewModel.manufacturer
            model: page.dashboardViewModel.model
            androidVersion: page.dashboardViewModel.androidVersion
            apiLevel: page.dashboardViewModel.apiLevel
            visible: page.dashboardViewModel.connected
        }

        GridLayout {
            Layout.fillWidth: true
            columns: page.availableWidth >= 820 ? 4 : page.availableWidth >= 420 ? 2 : 1
            columnSpacing: 12
            rowSpacing: 12
            visible: page.dashboardViewModel.connected
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("Battery"); value: page.dashboardViewModel.connected ? qsTr("%1%").arg(page.dashboardViewModel.batteryPercent) : "" }
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("Storage"); value: page.dashboardViewModel.storage }
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("CPU"); value: page.dashboardViewModel.cpu }
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("Memory"); value: page.dashboardViewModel.memory }
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("Brand"); value: page.dashboardViewModel.brand }
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("Build number"); value: page.dashboardViewModel.buildNumber }
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("OEM skin"); value: page.dashboardViewModel.oemSkin }
        }

        GroupBox {
            title: qsTr("Connections")
            Layout.fillWidth: true
            visible: page.dashboardViewModel.connected
            RowLayout {
                anchors.fill: parent
                Components.StatusBadge { text: qsTr("USB: Disconnected"); statusColor: page.themeService.error }
                Components.StatusBadge { text: qsTr("Wireless: Disconnected"); statusColor: page.themeService.error }
                Item { Layout.fillWidth: true }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: page.availableWidth >= 640 ? 2 : 1
            GroupBox {
                title: qsTr("Connection guidance")
                Layout.fillWidth: true
                Label {
                    anchors.fill: parent
                    text: page.dashboardViewModel.recommendedAction
                    color: palette.placeholderText
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
