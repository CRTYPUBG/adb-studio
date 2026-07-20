import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.FluentWinUI3

Button {
    highlighted: true
    Layout.minimumWidth: implicitWidth
    Accessible.role: Accessible.Button
    Accessible.name: text
}
