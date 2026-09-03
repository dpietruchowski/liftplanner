import QtQuick
import QtQuick.Controls
import LiftPlanner 1.0
import Themed.Components

WorkoutCard {
    id: root

    signal startWorkoutRequest(var workout)

    borderColor: Theme.colors.primaryBorder
    dateColor: Theme.colors.primary
    dateText: workout ? Qt.formatDateTime(workout.plannedTime, "ddd, d MMM yyyy") : ""

    ThemedButton {
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
