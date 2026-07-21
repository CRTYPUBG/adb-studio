pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3
import ADBStudio.Presentation
import "../components" as Components

Item {
    id: page
    required property ThemeService themeService
    required property WorkspaceViewModel workspaceViewModel
    Accessible.role: Accessible.Pane
    Accessible.name: page.workspaceViewModel.title

    Shortcut {
        sequences: [StandardKey.Refresh]
        onActivated: page.workspaceViewModel.refresh()
    }
    Shortcut {
        sequences: [StandardKey.Undo]
        enabled: page.workspaceViewModel.canUndo
        onActivated: page.workspaceViewModel.undo()
    }
    Shortcut {
        sequence: "Ctrl+F"
        onActivated: moduleSearch.forceActiveFocus()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        ToolBar {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                ColumnLayout {
                    Layout.fillWidth: true
                    Label {
                        text: page.workspaceViewModel.title
                        font.pixelSize: 26
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: page.workspaceViewModel.description
                        color: palette.placeholderText
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
                Button {
                    text: qsTr("Undo")
                    enabled: page.workspaceViewModel.canUndo
                    Accessible.description: qsTr("Return to the previous workspace")
                    onClicked: page.workspaceViewModel.undo()
                }
                Components.PrimaryButton {
                    text: qsTr("Refresh")
                    Accessible.description: qsTr("Refresh the current workspace")
                    onClicked: page.workspaceViewModel.refresh()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: moduleSearch
                Layout.fillWidth: true
                placeholderText: qsTr("Search capabilities")
                text: page.workspaceViewModel.searchText
                Accessible.name: qsTr("Search current workspace")
                onTextEdited: page.workspaceViewModel.searchText = text
            }
            ComboBox {
                id: filterBox
                Layout.preferredWidth: 190
                model: page.workspaceViewModel.filters
                Accessible.name: qsTr("Capability filter")
                onActivated: page.workspaceViewModel.activeFilter = currentText
            }
        }

        Frame {
            Layout.fillWidth: true
            visible: page.workspaceViewModel.errorMessage.length > 0
            Accessible.role: Accessible.AlertMessage
            RowLayout {
                anchors.fill: parent
                Label {
                    Layout.fillWidth: true
                    text: page.workspaceViewModel.errorMessage
                    color: page.themeService.error
                    wrapMode: Text.WordWrap
                }
                Button {
                    text: qsTr("Dismiss")
                    onClicked: page.workspaceViewModel.clearError()
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth
            GridLayout {
                width: parent.width
                columns: page.width >= 920 ? 3 : page.width >= 560 ? 2 : 1
                columnSpacing: 12
                rowSpacing: 12

                Repeater {
                    model: page.workspaceViewModel.features
                    delegate: Frame {
                        id: featureCard
                        required property string id
                        required property string title
                        required property string category
                        Layout.fillWidth: true
                        Layout.minimumHeight: 136
                        Accessible.role: Accessible.ListItem
                        Accessible.name: featureCard.title
                        Accessible.description: featureCard.category

                        ColumnLayout {
                            anchors.fill: parent
                            Label {
                                text: featureCard.title
                                font.pixelSize: 17
                                font.weight: Font.DemiBold
                            }
                            Components.StatusBadge {
                                text: qsTr("Available")
                                statusColor: page.themeService.success
                            }
                            Label {
                                Layout.fillWidth: true
                                text: featureCard.category
                                color: palette.placeholderText
                                wrapMode: Text.WordWrap
                            }
                            Item { Layout.fillHeight: true }
                            Button {
                                text: qsTr("Run")
                                onClicked: page.workspaceViewModel.activateFeature(featureCard.id)
                            }
                        }

                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: featureMenu.popup()
                        }
                        Menu {
                            id: featureMenu
                            MenuItem {
                                text: qsTr("Run capability")
                                onTriggered: page.workspaceViewModel.activateFeature(featureCard.id)
                            }
                            MenuItem {
                                text: qsTr("Refresh workspace")
                                onTriggered: page.workspaceViewModel.refresh()
                            }
                        }
                    }
                }
            }
        }

        Label {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            visible: page.workspaceViewModel.empty && !page.workspaceViewModel.busy
            text: qsTr("No capabilities match the current search and filter.")
            Accessible.name: text
        }

        ToolBar {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                Label { text: page.workspaceViewModel.statusText }
                Item { Layout.fillWidth: true }
                Label { text: qsTr("%1 capabilities").arg(page.workspaceViewModel.features.length) }
            }
        }
    }

    Components.LoadingView {
        anchors.fill: parent
        visible: page.workspaceViewModel.busy
        message: qsTr("Refreshing workspace")
    }

    opacity: page.workspaceViewModel.busy ? 0.88 : 1.0
    Behavior on opacity {
        enabled: !page.themeService.reducedMotion
        NumberAnimation { duration: 120 }
    }
}
