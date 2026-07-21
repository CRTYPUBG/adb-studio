pragma ComponentBehavior: Bound
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
    required property LanguageService languageService
    required property DeviceDashboardViewModel dashboardViewModel
    required property WorkspaceViewModel workspaceViewModel
    required property MirrorViewModel mirrorViewModel
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
                placeholderText: qsTr("Search current workspace")
                Accessible.name: qsTr("Search ADB Studio")
                text: window.workspaceViewModel.searchText
                onTextEdited: window.workspaceViewModel.searchText = text
            }
            ComboBox {
                id: languageSelector
                model: window.languageService.languages
                textRole: "label"
                valueRole: "code"
                currentIndex: window.languageService.languageCode === "tr" ? 1 : 0
                Accessible.name: qsTr("Application language")
                onActivated: window.languageService.languageCode = currentValue
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
            Layout.preferredWidth: window.width < 900 ? 92 : 232
            ColumnLayout {
                anchors.fill: parent
                spacing: 4
                Label {
                    text: window.width < 900 ? qsTr("Modules") : qsTr("Workspaces")
                    font.weight: Font.DemiBold
                    Layout.leftMargin: 10
                }
                ListView {
                    id: navigationList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 2
                    model: window.workspaceViewModel.modules
                    currentIndex: 0
                    delegate: ItemDelegate {
                        id: navigationItem
                        required property string key
                        required property string shortTitle
                        required property string title
                        required property int index
                        width: ListView.view.width
                        text: window.width < 900 ? navigationItem.shortTitle : navigationItem.title
                        highlighted: window.workspaceViewModel.currentModuleKey === navigationItem.key
                        Accessible.name: navigationItem.title
                        Accessible.description: qsTr("Open %1 workspace").arg(navigationItem.title)
                        onClicked: {
                            navigationList.currentIndex = navigationItem.index
                            window.workspaceViewModel.selectModule(navigationItem.key)
                        }
                    }
                    ScrollBar.vertical: ScrollBar { }
                }
            }
        }

        Pages.DashboardPage {
            themeService: window.themeService
            dashboardViewModel: window.dashboardViewModel
            workspaceViewModel: window.workspaceViewModel
            deviceImageSource: "qrc:/assets/logo-adb.png"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 24
            visible: window.workspaceViewModel.currentModuleKey === "dashboard"
        }

        Pages.WorkspaceModulePage {
            themeService: window.themeService
            workspaceViewModel: window.workspaceViewModel
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 18
            visible: window.workspaceViewModel.currentModuleKey !== "dashboard"
                     && window.workspaceViewModel.currentModuleKey !== "mirror"
        }

        Pages.MirrorPage {
            themeService: window.themeService
            mirrorViewModel: window.mirrorViewModel
            workspaceViewModel: window.workspaceViewModel
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 18
            visible: window.workspaceViewModel.currentModuleKey === "mirror"
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

    Popup {
        id: notificationPopup
        x: (parent.width - width) / 2
        y: parent.height - height - 72
        modal: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        visible: window.workspaceViewModel.notification.length > 0
        RowLayout {
            Accessible.role: Accessible.AlertMessage
            Label {
                Layout.maximumWidth: 560
                text: window.workspaceViewModel.notification
                wrapMode: Text.WordWrap
            }
            Button {
                text: qsTr("Dismiss")
                onClicked: window.workspaceViewModel.clearNotification()
            }
        }
    }
}
