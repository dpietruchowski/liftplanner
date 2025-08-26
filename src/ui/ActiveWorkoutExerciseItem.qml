import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

ColumnLayout {
    property var exercise
    width: parent.width
    spacing: Theme.spacing / 2

    Rectangle {
        Layout.fillWidth: true
        height: 50
        radius: Theme.borderRadius
        color: Theme.primary
        border.color: Theme.primaryVariant
        border.width: 1

        Text {
            text: exercise.name
            color: Theme.buttonText
            font.pixelSize: Theme.fontMedium
            font.bold: true
            anchors.centerIn: parent
        }
    }

    ColumnLayout {
        width: parent.width
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