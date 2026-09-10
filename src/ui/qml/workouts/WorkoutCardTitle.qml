import QtQuick
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

ColumnLayout {
    id: root

    property string name: ""
    property string dateText: ""
    property color dateColor: Theme.colors.textMuted
    property string titleName: ""
    property string dateName: ""

    spacing: 2

    Text {
        objectName: root.titleName
        Layout.fillWidth: true
        text: root.name
        font.pixelSize: Theme.fontSize.medium
        font.bold: true
        color: Theme.colors.textPrimary
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideRight
    }

    Text {
        objectName: root.dateName
        Layout.fillWidth: true
        text: root.dateText
        font.pixelSize: Theme.fontSize.small
        color: root.dateColor
        elide: Text.ElideRight
        visible: text.length > 0
    }
}
