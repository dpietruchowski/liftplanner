import QtQuick
import QtQuick.Layouts
import Themed.Components

Rectangle {
    id: root

    property var definition
    property bool current: false

    signal clicked()

    Layout.fillWidth: true
    implicitHeight: content.implicitHeight + Theme.padding.medium * 2
    radius: Theme.radius.medium
    color: mouseArea.pressed ? Theme.colors.surfaceAccent : Theme.colors.cardBackground
    border.width: root.current ? Theme.border.medium : Theme.border.thin
    border.color: root.current ? Theme.colors.primary : Theme.colors.cardBorder

    Behavior on color { ColorAnimation { duration: 120 } }

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: Theme.padding.medium
        spacing: Theme.spacing.xSmall

        Text {
            objectName: "exerciseName"
            Layout.fillWidth: true
            text: root.definition ? root.definition.name : ""
            color: Theme.colors.textPrimary
            font.pixelSize: Theme.fontSize.medium
            elide: Text.ElideRight
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.small

            Repeater {
                model: root.definition ? root.definition.primaryMuscles : []

                delegate: Rectangle {
                    height: Theme.chip.height
                    width: chipLabel.implicitWidth + Theme.chip.padding * 2
                    radius: Theme.chip.radius
                    color: Theme.colors.chipBackground

                    Text {
                        id: chipLabel
                        anchors.centerIn: parent
                        text: modelData
                        color: Theme.colors.chipText
                        font.pixelSize: Theme.chip.fontSize
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: root.definition ? root.definition.equipment : ""
                color: Theme.colors.textMuted
                font.pixelSize: Theme.fontSize.small
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
