import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3

ColumnLayout {
    property string message: qsTr("Loading")
    spacing: 12
    BusyIndicator {
        running: parent.visible
        Layout.alignment: Qt.AlignHCenter
        Accessible.name: parent.message
    }
    Label {
        text: parent.message
        Layout.alignment: Qt.AlignHCenter
    }
}
