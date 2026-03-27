import QtQuick
import Themed.Components

Rectangle {
    id: root

    property string label: ""
    property int value: 0

    width: 120
    height: 64
    radius: Theme.radius.medium
    color: Theme.colors.surface

    Column {
        anchors.centerIn: parent
        spacing: 2

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.value
            color: Theme.colors.textPrimary
            font.pixelSize: Theme.fontSize.xlarge
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            color: Theme.colors.textSecondary
            font.pixelSize: Theme.fontSize.small
        }
    }
}
