// ActiveWorkoutExerciseItem.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0

Column {
    id: exerciseDelegate
    property var exercise: modelData
    signal editSetRepetitions(var setData)
    signal editSetWeight(var setData)
    signal showExerciseInfo(var exercise)
    width: contentColumn.width
    spacing: Theme.spacing / 2

    Rectangle {
        width: exerciseDelegate.width
        height: 50
        radius: Theme.borderRadius
        color: Theme.primary
        border.color: exercise.completed ? Theme.success : Theme.border
        border.width: 2

        Text {
            text: exercise.name
            color: Theme.buttonText
            font.pixelSize: Theme.fontMedium
            font.bold: true
            anchors.centerIn: parent
            elide: Text.ElideRight
        }

        PrimaryButton {
            anchors.right: parent.right
            anchors.rightMargin: Theme.spacing
            anchors.verticalCenter: parent.verticalCenter
            svgIcon: Theme.icons.info
            circular: true
            buttonTheme: Theme.buttonSquare
            buttonStyle: Theme.buttonStylePrimary
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
        spacing: Theme.spacing / 2
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
