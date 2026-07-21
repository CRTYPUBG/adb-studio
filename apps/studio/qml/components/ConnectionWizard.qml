pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3
import ADBStudio.Presentation

ColumnLayout {
    id: wizard
    required property ThemeService themeService
    required property DeviceDashboardViewModel dashboardViewModel
    spacing: 16
    Accessible.role: Accessible.Pane
    Accessible.name: qsTr("Smart connection wizard")

    Frame {
        Layout.fillWidth: true
        ColumnLayout {
            anchors.fill: parent
            spacing: 12

            Image {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 112
                Layout.preferredHeight: 112
                source: "qrc:/assets/logo-adb.png"
                fillMode: Image.PreserveAspectFit
                Accessible.name: qsTr("USB device detection")
                SequentialAnimation on opacity {
                    running: wizard.dashboardViewModel.scanning && !wizard.themeService.reducedMotion
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.45; duration: 600 }
                    NumberAnimation { to: 1.0; duration: 600 }
                }
            }

            Label {
                Layout.fillWidth: true
                text: wizard.dashboardViewModel.state
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 24
                font.weight: Font.DemiBold
                wrapMode: Text.Wrap
            }
            Label {
                Layout.fillWidth: true
                text: wizard.dashboardViewModel.explanation
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                color: palette.placeholderText
            }
            ProgressBar {
                Layout.fillWidth: true
                indeterminate: wizard.dashboardViewModel.scanning
                value: wizard.dashboardViewModel.scanning ? 0 : 1
                Accessible.name: wizard.dashboardViewModel.currentDetectionStep
            }
            Label {
                Layout.fillWidth: true
                text: wizard.dashboardViewModel.currentDetectionStep
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
            }
            Label {
                Layout.fillWidth: true
                text: wizard.dashboardViewModel.recommendedAction
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                font.weight: Font.Medium
            }
            GridLayout {
                Layout.fillWidth: true
                columns: wizard.width < 800 ? 2 : 5
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Scan devices")
                    enabled: !wizard.dashboardViewModel.scanning
                    Accessible.description: qsTr("Run verified ADB and transport detection")
                    onClicked: wizard.dashboardViewModel.refresh()
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Restart ADB and rescan")
                    enabled: !wizard.dashboardViewModel.scanning
                    Accessible.description: qsTr("Restart ADB, clear stale sessions, and rescan")
                    onClicked: wizard.dashboardViewModel.recoverConnection()
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Open OEM guide")
                    enabled: wizard.dashboardViewModel.oemDetected
                    onClicked: wizard.dashboardViewModel.openOemGuide()
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Wireless setup")
                    enabled: !wizard.dashboardViewModel.scanning
                    onClicked: wizard.dashboardViewModel.openWirelessSetup()
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Install driver")
                    enabled: !wizard.dashboardViewModel.scanning
                    onClicked: wizard.dashboardViewModel.openDriverGuide()
                }
            }
        }
    }

    GroupBox {
        title: qsTr("Device health")
        Layout.fillWidth: true
        ColumnLayout {
            anchors.fill: parent
            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: qsTr("%1 / 100").arg(wizard.dashboardViewModel.healthScore)
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: qsTr("Evidence confidence: %1%").arg(wizard.dashboardViewModel.healthConfidence)
                    color: palette.placeholderText
                }
            }
            ProgressBar {
                Layout.fillWidth: true
                from: 0
                to: 100
                value: wizard.dashboardViewModel.healthScore
                Accessible.name: qsTr("Device health score")
                Accessible.description: qsTr("Score based only on measured diagnostics")
            }
        }
    }

    GroupBox {
        title: qsTr("Smart diagnostics")
        Layout.fillWidth: true
        ColumnLayout {
            anchors.fill: parent
            Repeater {
                model: wizard.dashboardViewModel.diagnostics
                delegate: Frame {
                    id: diagnosticDelegate
                    required property var modelData
                    Layout.fillWidth: true
                    Accessible.role: Accessible.ListItem
                    Accessible.name: modelData.title + ": " + modelData.status
                    RowLayout {
                        anchors.fill: parent
                        StatusBadge {
                            text: diagnosticDelegate.modelData.status
                            statusColor: diagnosticDelegate.modelData.status === "PASS"
                                         ? wizard.themeService.success
                                         : diagnosticDelegate.modelData.status === "FAIL"
                                           ? wizard.themeService.error
                                           : wizard.themeService.warning
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            Label { text: diagnosticDelegate.modelData.title; font.weight: Font.DemiBold }
                            Label {
                                Layout.fillWidth: true
                                text: diagnosticDelegate.modelData.summary
                                wrapMode: Text.Wrap
                                color: palette.placeholderText
                            }
                            Label {
                                Layout.fillWidth: true
                                text: diagnosticDelegate.modelData.action
                                wrapMode: Text.Wrap
                                visible: text.length > 0
                            }
                            Label {
                                Layout.fillWidth: true
                                text: diagnosticDelegate.modelData.why
                                wrapMode: Text.Wrap
                                color: palette.placeholderText
                            }
                        }
                        ColumnLayout {
                            Label {
                                text: qsTr("%1 min · %2").arg(diagnosticDelegate.modelData.minutes)
                                      .arg(diagnosticDelegate.modelData.difficulty)
                                color: palette.placeholderText
                            }
                            Button {
                                text: qsTr("Fix automatically")
                                visible: diagnosticDelegate.modelData.automaticFix
                                enabled: !wizard.dashboardViewModel.scanning
                                onClicked: wizard.dashboardViewModel.recoverConnection()
                            }
                        }
                    }
                }
            }
        }
    }

    GroupBox {
        title: wizard.dashboardViewModel.oemDetected
               ? wizard.dashboardViewModel.oemGuideTitle
               : qsTr("Manufacturer guide")
        Layout.fillWidth: true
        visible: wizard.dashboardViewModel.oemDetected
        ColumnLayout {
            anchors.fill: parent
            Label {
                text: qsTr("Estimated completion time: %1 minutes")
                      .arg(wizard.dashboardViewModel.oemGuideMinutes)
                color: palette.placeholderText
            }
            Repeater {
                model: wizard.dashboardViewModel.oemGuideSteps
                delegate: Label {
                    required property string modelData
                    required property int index
                    Layout.fillWidth: true
                    text: qsTr("%1. %2").arg(index + 1).arg(modelData)
                    wrapMode: Text.Wrap
                }
            }
        }
    }
}
