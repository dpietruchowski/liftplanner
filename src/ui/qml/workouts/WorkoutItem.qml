import QtQuick
import QtQuick.Controls
import LiftPlanner 1.0
import Themed.Components

WorkoutCard {
    id: root

    property int itemIndex: 0

    signal deleteWorkout(var workout)
    signal exportWorkout(var workout)
    signal repeatWorkout(var workout)

    objectName: "historyWorkoutItem" + root.itemIndex
    expandButtonName: "historyWorkoutExpandButton" + root.itemIndex
    titleName: "historyWorkoutTitle" + root.itemIndex
    chipPrefix: "historyWorkoutSetChip" + root.itemIndex
    dateText: workout ? Qt.formatDateTime(workout.startedTime, "ddd, d MMM yyyy") : ""

    expandedActions: Component {
        Row {
            spacing: Theme.spacing.medium

            ThemedButton {
                objectName: "historyWorkoutRepeatButton" + root.itemIndex
                iconSource: Theme.icons.curvedArrow
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.success
                onClicked: root.repeatWorkout(root.workout)
                ToolTip.visible: hovered
                ToolTip.text: "Do this workout again"
                ToolTip.delay: 500
            }

            ThemedButton {
                objectName: "historyWorkoutCopyButton" + root.itemIndex
                iconSource: Theme.icons.copy
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.tonal
                onClicked: root.exportWorkout(root.workout)
                ToolTip.visible: hovered
                ToolTip.text: "Copy to clipboard"
                ToolTip.delay: 500
            }

            ThemedButton {
                objectName: "historyWorkoutDeleteButton" + root.itemIndex
                iconSource: Theme.icons.close
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.danger
                onClicked: root.deleteWorkout(root.workout)
                ToolTip.visible: hovered
                ToolTip.text: "Delete workout"
                ToolTip.delay: 500
            }
        }
    }
}
