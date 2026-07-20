import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3

Pane {
    id: control
    property string manufacturer
    property string model
    property string androidVersion
    property int apiLevel
    property bool connected
    property url deviceImageSource
    Accessible.role: Accessible.Grouping
    Accessible.name: connected ? manufacturer + " " + model : qsTr("No connected device")

    background: Rectangle {
        radius: 8
        color: control.palette.base
        border.color: control.palette.midlight
    }

    RowLayout {
        anchors.fill: parent
        spacing: 16
        Image {
            Layout.preferredWidth: 62
            Layout.preferredHeight: 96
            source: control.deviceImageSource
            fillMode: Image.PreserveAspectFit
            Accessible.ignored: true
        }
        ColumnLayout {
            Layout.fillWidth: true
            Label {
                text: control.connected ? control.manufacturer : qsTr("Connect an Android device")
                font.pixelSize: 20
                font.weight: Font.DemiBold
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            Label {
                text: control.connected ? control.model : qsTr("USB and wireless devices will appear here.")
                color: control.palette.placeholderText
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Label {
                visible: control.connected
                text: qsTr("Android %1 - API %2").arg(control.androidVersion).arg(control.apiLevel)
                color: control.palette.placeholderText
            }
        }
    }
}
