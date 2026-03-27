import QtQuick
import QtQuick.Layouts
import Themed.Components

Rectangle {
    id: root

    property string label: ""
    property string workoutName: ""
    property string dateText: ""
    property int nameSize: Theme.fontSize.medium
    property color nameColor: Theme.colors.textSecondary
    property bool cardVisible: false

    radius: Theme.radius.large
    color: cardVisible ? Theme.colors.surface : "transparent"
    border.width: cardVisible ? Theme.border.medium : 0
    border.color: Theme.colors.primaryVariant

    implicitHeight: inner.implicitHeight

    Column {
        id: inner
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: Theme.padding.medium
        }
        spacing: 2

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            color: Theme.colors.textSecondary
            font.pixelSize: Theme.fontSize.small
            visible: root.label.length > 0
        }

        RowLayout {
            width: parent.width
            spacing: Theme.spacing.small

            Text {
                text: root.dateText
                color: Theme.colors.textSecondary
                font.pixelSize: Theme.fontSize.small
                visible: root.dateText.length > 0
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                text: root.workoutName
                color: root.nameColor
                font.pixelSize: root.nameSize
                font.bold: true
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                Layout.alignment: Qt.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
