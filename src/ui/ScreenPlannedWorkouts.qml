import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    color: Theme.colors.background
    property var currentWorkout

    Component.onCompleted: PlannedWorkoutService.loadAll()

    WorkoutListView {
        anchors.fill: parent
        workouts: PlannedWorkoutService.workouts
        workoutDelegate: PlannedWorkoutItem {
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
        buttonsRow: WorkoutButtonsRow {
            onImportWorkouts: importPopup.open()
        }
    }

    function startWorkout() {
        if (!currentWorkout)
            return
        ActiveWorkoutService.startWorkout(currentWorkout)
        PlannedWorkoutService.loadAll()
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

    NotificationPopup {
        id: importPopup
        text: "Do you want to import new planned workouts?"
        type: Notification.Type.Warning
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            PlannedWorkoutService.importFromClipboard()
        }
    }
}
