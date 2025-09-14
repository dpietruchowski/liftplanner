import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0

Rectangle {
    id: root
    color: Theme.background
    property var currentWorkout

    Component.onCompleted: RoutineService.loadAllWorkouts()

    WorkoutListView {
        anchors.fill: parent
        workouts: RoutineService.workouts
        workoutDelegate: RoutineItem {
            workout: modelData
            onStartWorkoutRequest: function(workout) {
                root.currentWorkout = workout
                if (ActiveWorkoutService.currentWorkout) {
                     startWorkoutPopup.open()
                 } else {
                     startWorkout()
                 }
            }
        }
        buttonsRow: WorkoutButtonsRow {}
    }

    function startWorkout() {
        if (!currentWorkout)
            return
        ActiveWorkoutService.startWorkout(currentWorkout)
        if (stackView.currentItem !== activeWorkoutScreen) {
            stackView.replace(activeWorkoutScreen)
        }
    }

    NotificationPopup {
        id: startWorkoutPopup
        text: "Previous workout was not ended. Do you want to start new one?"
        type: Notification.Type.Warning
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            startWorkout()
        }
    }
}
