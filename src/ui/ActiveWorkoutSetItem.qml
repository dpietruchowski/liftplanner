// ActiveWorkoutSetItem.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

Rectangle {
    id: setRect
    property var setData
    signal editRepetitions(var setData)
    signal editWeight(var setData)
    width: setsColumn.width
    height: 60
    radius: Theme.borderRadius
    color: setData.completed ? Theme.success : Theme.surface
    border.width: ActiveWorkoutService.currentSet === setData ? 3 : 1
    border.color: ActiveWorkoutService.currentSet === setData ? Theme.primaryVariant : Theme.border

    Behavior on color { ColorAnimation { duration: 200 } }


    MouseArea {
        anchors.fill: parent
        onClicked: function(mouse) {
            ActiveWorkoutService.currentSet = setData
        }
    }
    Row {
        anchors.fill: parent
        anchors.margins: Theme.padding
        spacing: Theme.spacing

        Text {
            text: "Set " + (index + 1)
            font.pixelSize: Theme.fontSmall
            font.bold: true
            color: setData.completed ? Theme.buttonText : Theme.textPrimary
            width: 60
        }

        Item {
            width: 80
            height: parent.height

            Text {
                text: setData.repetitions + " reps"
                font.pixelSize: Theme.fontSmall
                color: setData.completed ? Theme.buttonText : Theme.textSecondary
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
            width: 80
            height: parent.height

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    ActiveWorkoutService.currentSet = setData
                    editWeight(setData)
                }
            }

            Text {
                id: setText
                text: setData.weight + " kg"
                font.pixelSize: Theme.fontSmall
                color: setData.completed ? Theme.buttonText : Theme.textSecondary
                anchors.centerIn: parent
            }
        }

        Item {
            width: Math.max(0, parent.width - (60 + 80 + 80 + Theme.spacing * 2 + Theme.padding * 2 + 28))
        }

        Rectangle {
            width: 20
            height: 20
            radius: 10
            color: setData.completed ? Theme.buttonText : "transparent"
            border.color: setData.completed ? Theme.success : Theme.border
            border.width: 2
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
