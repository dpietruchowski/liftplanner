import QtQuick
import QtQuick.Controls
import LiftPlanner 1.0
import Themed.Components

WorkoutCard {
    id: root

    property int itemIndex: 0

    signal startWorkoutRequest(var workout)
    signal editWorkoutRequest(var workout)
    signal deleteWorkoutRequest(var workout)

    objectName: "plannedWorkoutItem" + itemIndex
    expandButtonName: "plannedWorkoutExpandButton" + itemIndex
    titleName: "plannedWorkoutTitle" + itemIndex
    chipPrefix: "plannedWorkoutSetChip" + itemIndex
    borderColor: Theme.colors.primaryBorder
    dateColor: Theme.colors.primary
    dateText: workout ? Qt.formatDateTime(workout.plannedTime, "ddd, d MMM yyyy") : ""

    expandedActions: Component {
        Row {
            spacing: Theme.spacing.medium

            ThemedButton {
                objectName: "plannedWorkoutEditButton" + root.itemIndex
                iconSource: Theme.icons.edit
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.tonal
                onClicked: root.editWorkoutRequest(root.workout)
                ToolTip.visible: hovered
                ToolTip.text: "Edit workout"
                ToolTip.delay: 500
            }

            ThemedButton {
                objectName: "plannedWorkoutDeleteButton" + root.itemIndex
                iconSource: Theme.icons.close
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.danger
                onClicked: root.deleteWorkoutRequest(root.workout)
                ToolTip.visible: hovered
                ToolTip.text: "Delete workout"
                ToolTip.delay: 500
            }
        }
    }

    ThemedButton {
        objectName: "plannedWorkoutStartButton" + root.itemIndex
        iconSource: Theme.icons.startWorkout
        circular: true
        buttonSize: Theme.button.circle
        buttonStyle: Theme.button.primary
        onClicked: root.startWorkoutRequest(root.workout)
        ToolTip.visible: hovered
        ToolTip.text: "Start workout"
        ToolTip.delay: 500
    }
}
