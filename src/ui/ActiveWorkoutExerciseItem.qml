import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

Column {
    id: exerciseDelegate
    property var exercise: modelData
    width: contentColumn.width
    spacing: Theme.spacing / 2

    Rectangle {
        width: exerciseDelegate.width
        height: 50
        radius: Theme.borderRadius
        color: ActiveWorkoutService.currentExercise === exercise ? Theme.primary : Theme.surface
        border.color: ActiveWorkoutService.currentExercise === exercise ? Theme.primaryVariant : Theme.border
        border.width: 1

        Text {
            text: exercise.name
            color: Theme.buttonText
            font.pixelSize: Theme.fontMedium
            font.bold: true
            anchors.centerIn: parent
            elide: Text.ElideRight
        }

        MouseArea {
            anchors.fill: parent
            onClicked: ActiveWorkoutService.currentExercise = exercise
        }
    }

    Column {
        id: setsColumn
        width: exerciseDelegate.width
        spacing: Theme.spacing / 2
        visible: ActiveWorkoutService.currentExercise === exercise

        Repeater {
            model: exercise.sets

            delegate: ActiveWorkoutSetItem {
                setData: modelData
            }
        }
    }
    }
