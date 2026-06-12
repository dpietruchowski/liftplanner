import QtQuick
import QtQuick.Controls
import LiftPlanner 1.0
import Themed.Components

WorkoutCard {
    id: root

    signal startWorkoutRequest(var workout)

    borderColor: Theme.colors.primary
    dateText: workout ? Qt.formatDateTime(workout.plannedTime, "ddd, d MMM yyyy") : ""

    ThemedButton {
        iconSource: Theme.icons.startWorkout
        circular: true
        buttonSize: Theme.button.square
        buttonStyle: Theme.button.outline
        onClicked: root.startWorkoutRequest(root.workout)
        ToolTip.visible: hovered
        ToolTip.text: "Start workout"
        ToolTip.delay: 500
    }
}
