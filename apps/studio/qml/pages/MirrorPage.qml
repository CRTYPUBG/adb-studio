pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3
import QtQuick.Dialogs
import ADBStudio.Presentation
import "../components" as Components

ScrollView {
    id: page
    required property ThemeService themeService
    required property MirrorViewModel mirrorViewModel
    required property WorkspaceViewModel workspaceViewModel
    contentWidth: availableWidth
    Accessible.role: Accessible.Pane
    Accessible.name: qsTr("Screen mirroring")

    Component.onCompleted: page.mirrorViewModel.scanDevices()

    ColumnLayout {
        width: page.availableWidth
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                Label { text: qsTr("Screen mirroring"); font.pixelSize: 28; font.weight: Font.DemiBold }
                Label { text: page.mirrorViewModel.engineVersion; color: palette.placeholderText }
            }
            Components.StatusBadge {
                text: page.mirrorViewModel.active ? qsTr("Running") : page.mirrorViewModel.status
                statusColor: page.mirrorViewModel.active ? page.themeService.success : page.themeService.warning
            }
        }

        ToolBar {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                ComboBox {
                    id: deviceSelector
                    Layout.fillWidth: true
                    model: page.mirrorViewModel.devices
                    textRole: "label"
                    valueRole: "serial"
                    Accessible.name: qsTr("Mirror device")
                    onActivated: page.mirrorViewModel.selectedSerial = currentValue
                }
                Button {
                    text: qsTr("Scan")
                    enabled: !page.mirrorViewModel.scanning
                    onClicked: page.mirrorViewModel.scanDevices()
                }
                Components.PrimaryButton {
                    text: qsTr("Start")
                    enabled: !page.mirrorViewModel.active && page.mirrorViewModel.devices.length > 0
                    onClicked: page.mirrorViewModel.start()
                }
                Button {
                    text: qsTr("Stop")
                    enabled: page.mirrorViewModel.active
                    onClicked: page.mirrorViewModel.stop()
                }
                Button {
                    text: page.mirrorViewModel.paused ? qsTr("Resume") : qsTr("Pause")
                    enabled: page.mirrorViewModel.active
                    onClicked: page.mirrorViewModel.paused ? page.mirrorViewModel.resume()
                                                           : page.mirrorViewModel.pause()
                }
                Button {
                    text: qsTr("Reconnect")
                    enabled: page.mirrorViewModel.selectedSerial.length > 0
                    onClicked: page.mirrorViewModel.reconnect()
                }
            }
        }

        Frame {
            Layout.fillWidth: true
            visible: page.mirrorViewModel.errorCause.length > 0
            Accessible.role: Accessible.AlertMessage
            ColumnLayout {
                anchors.fill: parent
                Label { text: qsTr("Cause: %1").arg(page.mirrorViewModel.errorCause); color: page.themeService.error; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: qsTr("Solution: %1").arg(page.mirrorViewModel.errorSolution); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: page.mirrorViewModel.technicalDetails; visible: text.length > 0; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true; font.family: "Cascadia Mono" }
                RowLayout {
                    Button { text: qsTr("Retry"); onClicked: page.mirrorViewModel.reconnect() }
                    Button { text: qsTr("Repair and retry"); onClicked: page.mirrorViewModel.repairAndRetry() }
                    Button { text: qsTr("Diagnostics"); onClicked: page.workspaceViewModel.selectModule("guide") }
                    Button { text: qsTr("Open logs"); onClicked: page.contentItem.contentY = logsBox.y }
                    Button { text: qsTr("Copy error"); onClicked: page.mirrorViewModel.copyError() }
                    Button { text: qsTr("Report issue"); onClicked: page.mirrorViewModel.reportIssue() }
                    Button { text: qsTr("Dismiss"); onClicked: page.mirrorViewModel.clearError() }
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: page.availableWidth >= 860 ? 3 : page.availableWidth >= 520 ? 2 : 1
            columnSpacing: 12
            rowSpacing: 12

            GroupBox {
                title: qsTr("Video")
                Layout.fillWidth: true
                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    Label { text: qsTr("Resolution") }
                    ComboBox {
                        model: [qsTr("Original"), qsTr("720p"), qsTr("1080p"), qsTr("1440p")]
                        currentIndex: page.mirrorViewModel.maxSize === 0 ? 0 : page.mirrorViewModel.maxSize === 720 ? 1 : page.mirrorViewModel.maxSize === 1080 ? 2 : 3
                        onActivated: page.mirrorViewModel.maxSize = [0, 720, 1080, 1440][currentIndex]
                    }
                    Label { text: qsTr("Maximum FPS") }
                    SpinBox { from: 15; to: 240; value: page.mirrorViewModel.maxFps; onValueModified: page.mirrorViewModel.maxFps = value }
                    Label { text: qsTr("Bitrate (Mbps)") }
                    SpinBox { from: 1; to: 100; value: page.mirrorViewModel.bitRateMbps; onValueModified: page.mirrorViewModel.bitRateMbps = value }
                    Label { text: qsTr("Codec") }
                    ComboBox { model: ["h264", "h265", "av1"]; currentIndex: Math.max(0, model.indexOf(page.mirrorViewModel.codec)); onActivated: page.mirrorViewModel.codec = currentText }
                    Label { text: qsTr("Rotation") }
                    ComboBox { model: ["0°", "90°", "180°", "270°"]; currentIndex: page.mirrorViewModel.orientation / 90; onActivated: page.mirrorViewModel.orientation = currentIndex * 90 }
                }
            }

            GroupBox {
                title: qsTr("Session")
                Layout.fillWidth: true
                ColumnLayout {
                    anchors.fill: parent
                    Switch { text: qsTr("Forward audio"); checked: page.mirrorViewModel.audio; onToggled: page.mirrorViewModel.audio = checked }
                    Switch { text: qsTr("Start fullscreen"); checked: page.mirrorViewModel.fullscreen; onToggled: page.mirrorViewModel.fullscreen = checked }
                    Switch { text: qsTr("Clipboard synchronization"); checked: page.mirrorViewModel.clipboardSync; onToggled: page.mirrorViewModel.clipboardSync = checked }
                    Switch { text: qsTr("Automatic reconnect"); checked: page.mirrorViewModel.autoReconnect; onToggled: page.mirrorViewModel.autoReconnect = checked }
                    Label { text: qsTr("Active sessions: %1").arg(page.mirrorViewModel.activeSessions.length) }
                    Label { text: qsTr("Clipboard and APK drag-and-drop are handled by the verified scrcpy session."); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
            }

            GroupBox {
                title: qsTr("Performance overlay")
                Layout.fillWidth: true
                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    Label { text: qsTr("FPS") }
                    Label { text: page.mirrorViewModel.fps > 0 ? Number(page.mirrorViewModel.fps).toLocaleString(Qt.locale(), "f", 1) : qsTr("Measuring") }
                    Label { text: qsTr("ADB latency") }
                    Label { text: page.mirrorViewModel.latencyMs >= 0 ? qsTr("%1 ms").arg(page.mirrorViewModel.latencyMs) : qsTr("Measuring") }
                    Label { text: qsTr("Transport") }
                    Label { text: page.mirrorViewModel.selectedSerial.indexOf(":") >= 0 ? qsTr("Wireless") : qsTr("USB") }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Button { text: qsTr("Screenshot"); enabled: page.mirrorViewModel.selectedSerial.length > 0; onClicked: screenshotDialog.open() }
            Button { text: page.mirrorViewModel.recordingPath.length > 0 ? qsTr("Change recording file") : qsTr("Enable recording"); onClicked: recordingDialog.open() }
            Button { text: qsTr("Disable recording"); enabled: page.mirrorViewModel.recordingPath.length > 0; onClicked: page.mirrorViewModel.recordingPath = "" }
            Button { text: qsTr("Install APK"); enabled: page.mirrorViewModel.selectedSerial.length > 0; onClicked: apkDialog.open() }
            Item { Layout.fillWidth: true }
        }

        Frame {
            Layout.fillWidth: true
            Layout.minimumHeight: 84
            Label { anchors.centerIn: parent; text: qsTr("Drop an APK here to install it on the selected device"); Accessible.name: text }
            DropArea {
                anchors.fill: parent
                onDropped: drop => {
                    if (drop.urls.length > 0)
                        page.mirrorViewModel.installApk(drop.urls[0])
                }
            }
        }

        GroupBox {
            id: logsBox
            title: qsTr("Session logs")
            Layout.fillWidth: true
            TextArea {
                anchors.fill: parent
                readOnly: true
                text: page.mirrorViewModel.logs.join("\n")
                wrapMode: TextEdit.WrapAnywhere
                Accessible.name: qsTr("Mirror session logs")
            }
        }
    }

    FileDialog {
        id: screenshotDialog
        title: qsTr("Save device screenshot")
        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("PNG image (*.png)")]
        defaultSuffix: "png"
        onAccepted: page.mirrorViewModel.takeScreenshot(selectedFile)
    }
    FileDialog {
        id: recordingDialog
        title: qsTr("Choose recording file")
        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("MP4 video (*.mp4)"), qsTr("Matroska video (*.mkv)")]
        defaultSuffix: "mp4"
        onAccepted: page.mirrorViewModel.recordingPath = selectedFile
    }
    FileDialog {
        id: apkDialog
        title: qsTr("Select APK to install")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("Android package (*.apk)")]
        onAccepted: page.mirrorViewModel.installApk(selectedFile)
    }

    Components.LoadingView {
        anchors.fill: parent
        visible: page.mirrorViewModel.scanning
        message: qsTr("Scanning devices")
    }
}
