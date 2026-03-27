import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Column {
    id: exerciseDelegate
    property var exercise: modelData
    signal editSetRepetitions(var setData)
    signal editSetWeight(var setData)
    signal showExerciseInfo(var exercise)
    width: contentColumn.width
    spacing: Theme.spacing.medium / 2

    Rectangle {
        width: exerciseDelegate.width
        height: 50
        radius: Theme.radius.medium
        color: Theme.colors.primary
        border.color: exercise.completed ? Theme.colors.success : Theme.colors.border
        border.width: 2

        Text {
            text: exercise.name
            color: Theme.colors.buttonText
            font.pixelSize: Theme.fontSize.medium
            font.bold: true
            anchors.centerIn: parent
            elide: Text.ElideRight
        }

        ThemedButton {
            anchors.right: parent.right
            anchors.rightMargin: Theme.spacing.medium
            anchors.verticalCenter: parent.verticalCenter
            iconSource: Theme.icons.info
            circular: true
            buttonSize: Theme.button.square
            buttonStyle: Theme.button.primary
            z: 1
            onClicked: exerciseDelegate.showExerciseInfo(exercise)
        }

        MouseArea {
            anchors.fill: parent
            onClicked: ActiveWorkoutService.currentExercise = exercise
        }
    }

    Column {
        id: setsColumn
        width: exerciseDelegate.width
        spacing: Theme.spacing.medium / 2
        visible: ActiveWorkoutService.currentExercise === exercise

        Repeater {
            model: exercise.sets

            delegate: ActiveWorkoutSetItem {
                setData: modelData
                onEditRepetitions: function(setDat) { exerciseDelegate.editSetRepetitions(setData) }
                onEditWeight: function(setDat) { exerciseDelegate.editSetWeight(setData) }
            }
        }
    }
}
