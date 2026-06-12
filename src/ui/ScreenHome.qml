import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components
import App.Components

Rectangle {
    id: root
    color: Theme.colors.background

    anchors.margins: Theme.padding.medium

    ColumnLayout {
        width: parent.width - 2 * Theme.padding.medium
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 2

        // Top exercises (most frequent in recent workouts) with best 1RM
        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spacing.medium

            Repeater {
                model: WorkoutHistoryService.topExercises

                StatTile {
                    label: modelData.name
                    value: Math.round(modelData.oneRepMax) + " kg"
                }
            }
        }

        WeekActivityDots {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Theme.spacing.large
            Layout.bottomMargin: Theme.spacing.large
            activity: WorkoutHistoryService.weekActivity
            todayIndex: (new Date().getDay() + 6) % 7
        }

        // Next planned (smaller)
        WorkoutLabel {
            Layout.fillWidth: true
            implicitHeight: Theme.layout.cardHeight
            label: "planned workout"
            workout: PlannedWorkoutService.nextWorkout
        }

        // Current workout (large, prominent)
        WorkoutLabel {
            Layout.fillWidth: true
            implicitHeight: Theme.layout.cardHeight * 1.4
            cardVisible: true
            label: "current workout"
            nameSize: Theme.fontSize.xlarge
            nameColor: Theme.colors.textPrimary
            workout: ActiveWorkoutService.currentWorkout
        }

        // Last workout (smaller)
        WorkoutLabel {
            Layout.fillWidth: true
            implicitHeight: Theme.layout.cardHeight
            label: "last workout"
            workout: WorkoutHistoryService.lastWorkout
        }

        // Start workout button
        ThemedButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Theme.spacing.large
            text: "Start workout"
            buttonStyle: Theme.button.primary
            buttonSize: Theme.button.large
            onClicked: {
                if (ActiveWorkoutService.currentWorkout)
                    startWorkoutPopup.open()
                else if (!PlannedWorkoutService.nextWorkout)
                    noPlannedPopup.open()
                else
                    startWorkout()
            }
        }
    }

    NotificationPopup {
        id: noPlannedPopup
        text: "No planned workouts yet.\n\n" +
              "Head to the workouts tab (calendar icon) and tap the AI button — it will copy a ready-made prompt to your clipboard. " +
              "Paste it into any AI assistant (ChatGPT, Gemini, etc.), describe your training goals, and let it generate a workout plan. " +
              "Once you receive the JSON, come back and tap the import button next to 'Planned'."
        type: Notification.Type.Info
        buttons: Notification.Button.Ok
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

    function startWorkout() {
        if (!PlannedWorkoutService.nextWorkout)
            return
        ActiveWorkoutService.startWorkout(PlannedWorkoutService.nextWorkout)
        PlannedWorkoutService.loadAll()
        if (stackView.currentItem !== activeWorkoutScreen) {
            stackView.replace(activeWorkoutScreen)
        }
    }
}
