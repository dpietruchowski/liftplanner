import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    color: Theme.colors.background

    property var workoutToDelete

    WorkoutListView {
        anchors.fill: parent
        workouts: WorkoutHistoryService.workouts
        workoutDelegate: WorkoutItem {
            workout: modelData

            onDeleteWorkout: function(workout) {
                root.workoutToDelete = workout
                endWorkoutPopup.open()
            }
        }
    }

    NotificationPopup {
        id: endWorkoutPopup
        text: "Do you want to delete workout?"
        type: Notification.Type.Info
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            if (root.workoutToDelete) {
                WorkoutHistoryService.deleteWorkout(root.workoutToDelete)
                root.workoutToDelete = null
            }
        }
    }
}
