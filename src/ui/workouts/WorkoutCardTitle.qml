import QtQuick
import LiftPlanner 1.0
import Themed.Components

Column {
    id: root

    property string name: ""
    property string dateText: ""

    Text {
        text: root.name
        font.pixelSize: Theme.fontSize.medium
        font.bold: true
        color: Theme.colors.textPrimary
        elide: Text.ElideRight
        width: parent.width
    }

    Text {
        text: root.dateText
        font.pixelSize: Theme.fontSize.small
        color: Theme.colors.textSecondary
        visible: text.length > 0
    }
}
