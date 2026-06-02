import QtQuick
import Themed.Components

Rectangle {
    id: root

    property string label: ""
    property string value: ""

    width: 120
    height: 64
    radius: Theme.radius.medium
    color: Theme.colors.surface

    Column {
        anchors.centerIn: parent
        width: parent.width - 2 * Theme.padding.small
        spacing: 2

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: root.label
            color: Theme.colors.textSecondary
            font.pixelSize: Theme.fontSize.small
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.value
            color: Theme.colors.textPrimary
            font.pixelSize: Theme.fontSize.large
            font.bold: true
        }
    }
}
