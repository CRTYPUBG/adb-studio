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

        Components.DeviceCard {
            Layout.fillWidth: true
            connected: page.dashboardViewModel.connected
            deviceImageSource: page.deviceImageSource
            manufacturer: page.dashboardViewModel.manufacturer
            model: page.dashboardViewModel.model
            androidVersion: page.dashboardViewModel.androidVersion
            apiLevel: page.dashboardViewModel.apiLevel
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width >= 820 ? 4 : width >= 420 ? 2 : 1
            columnSpacing: 12
            rowSpacing: 12
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("Battery"); value: page.dashboardViewModel.connected ? qsTr("%1%").arg(page.dashboardViewModel.batteryPercent) : "" }
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("Storage"); value: page.dashboardViewModel.storage }
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("CPU"); value: page.dashboardViewModel.cpu }
            Components.InfoCard { Layout.fillWidth: true; title: qsTr("Memory"); value: page.dashboardViewModel.memory }
        }

        GroupBox {
            title: qsTr("Connections")
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                Components.StatusBadge { text: qsTr("USB: Disconnected"); statusColor: page.themeService.error }
                Components.StatusBadge { text: qsTr("Wireless: Disconnected"); statusColor: page.themeService.error }
                Item { Layout.fillWidth: true }
            }
        }
    }
}
