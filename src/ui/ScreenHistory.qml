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
                deletePopup.open()
            }
        }
        buttonsRow: RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.medium
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            Item { Layout.fillWidth: true }

            ThemedButton {
                text: "Import"
                buttonSize: Theme.button.medium
                buttonStyle: Theme.button.primary
                onClicked: importPopup.open()
            }
        }
    }

    NotificationPopup {
        id: deletePopup
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

    NotificationPopup {
        id: importPopup
        text: "Import workout history from clipboard?"
        type: Notification.Type.Info
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            WorkoutHistoryService.importFromClipboard()
        }
    }
}
