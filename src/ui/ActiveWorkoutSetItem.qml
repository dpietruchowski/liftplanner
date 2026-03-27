import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: setRect
    property var setData
    property bool actionsVisible: false
    signal editRepetitions(var setData)
    signal editWeight(var setData)

    width: setsColumn.width
    height: actionsVisible ? 100 : 60
    radius: Theme.radius.medium
    color: setData.completed ? Theme.colors.success : Theme.colors.surface
    border.width: ActiveWorkoutService.currentSet === setData ? 3 : 1
    border.color: ActiveWorkoutService.currentSet === setData ? Theme.colors.primaryVariant : Theme.colors.border

    Behavior on color { ColorAnimation { duration: 200 } }

    MouseArea {
        anchors.fill: parent
        onClicked: ActiveWorkoutService.currentSet = setData
    }

    Column {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        RowLayout {
            id: mainRow
            width: parent.width
            height: 60
            spacing: Theme.spacing.medium
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Theme.padding.medium

            Text {
                text: "Set " + (index + 1)
                font.pixelSize: Theme.fontSize.small
                font.bold: true
                color: setData.completed ? Theme.colors.buttonText : Theme.colors.textPrimary
                Layout.preferredWidth: 60
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                Text {
                    text: setData.repetitions + " reps"
                    font.pixelSize: Theme.fontSize.small
                    color: setData.completed ? Theme.colors.buttonText : Theme.colors.textSecondary
                    anchors.centerIn: parent
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        ActiveWorkoutService.currentSet = setData
                        editRepetitions(setData)
                    }
                }
            }

            Item {
                Layout.preferredWidth: 80
                Layout.fillHeight: true

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        ActiveWorkoutService.currentSet = setData
                        editWeight(setData)
                    }
                }

                Text {
                    text: setData.weight + " kg"
                    font.pixelSize: Theme.fontSize.small
                    color: setData.completed ? Theme.colors.buttonText : Theme.colors.textSecondary
                    anchors.centerIn: parent
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: setData.completed ? Theme.colors.buttonText : "transparent"
                border.color: setData.completed ? Theme.colors.success : Theme.colors.border
                border.width: 2
                Layout.alignment: Qt.AlignVCenter

                MouseArea {
                    anchors.fill: parent
                    onClicked: actionsVisible = !actionsVisible
                }
            }
        }

        RowLayout {
            id: actionsRow
            spacing: Theme.spacing.medium
            visible: actionsVisible
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Theme.padding.medium

            Item { Layout.fillWidth: true }

            ThemedButton {
                 iconSource: Theme.icons.addSet
                 circular: true
                 buttonSize: Theme.button.square
                 buttonStyle: Theme.button.primary
                 onClicked: ActiveWorkoutService.duplicateSet(modelData)
            }

            ThemedButton {
                 iconSource: Theme.icons.removeSet
                 circular: true
                 buttonSize: Theme.button.square
                 buttonStyle: Theme.button.primary
                 onClicked: ActiveWorkoutService.removeSet(modelData)
            }
        }
    }
}
