import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3

Label {
    id: control
    property color statusColor: palette.accent
    Layout.minimumWidth: implicitWidth
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    leftPadding: 10
    rightPadding: 10
    topPadding: 4
    bottomPadding: 4
    Accessible.role: Accessible.StaticText
    Accessible.name: text

    background: Rectangle {
        radius: 6
        color: Qt.rgba(control.statusColor.r, control.statusColor.g, control.statusColor.b, 0.14)
        border.color: Qt.rgba(control.statusColor.r, control.statusColor.g, control.statusColor.b, 0.5)
    }
}
