import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3

Pane {
    id: control
    property string title
    property string value
    Accessible.role: Accessible.Grouping
    Accessible.name: title
    implicitWidth: 190
    implicitHeight: 94

    background: Rectangle {
        radius: 8
        color: control.palette.base
        border.color: control.palette.midlight
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6
        Label {
            text: control.title
            color: control.palette.placeholderText
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
        Label {
            text: control.value.length > 0 ? control.value : qsTr("Unavailable")
            font.pixelSize: 18
            font.weight: Font.DemiBold
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
}
