import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    objectName: "screenHome"
    color: Theme.colors.background

    anchors.margins: Theme.padding.screen

    readonly property var drumEntries: {
        var list = []
        var planned = PlannedWorkoutViewModel.workouts
        var current = ActiveWorkoutViewModel.currentWorkout
        if (current) {
            if (planned.length > 0)
                list.push({ label: "planned workout", kind: "planned", workout: planned[0] })
            list.push({ label: "current workout", kind: "current", workout: current })
        } else {
            if (planned.length > 1)
                list.push({ label: "next planned", kind: "upcoming", workout: planned[1] })
            list.push({ label: "planned workout", kind: "planned",
                        workout: planned.length > 0 ? planned[0] : null })
        }
        list.push({ label: "last workout", kind: "last",
                    workout: WorkoutHistoryViewModel.lastWorkout })
        return list
    }

    ColumnLayout {
        width: parent.width - 2 * Theme.padding.screen
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0

        Text {
            objectName: "todayLabel"
            Layout.fillWidth: true
            text: Qt.formatDate(new Date(), "dddd, d MMM").toUpperCase()
            color: Theme.colors.textMuted
            font.pixelSize: Theme.fontSize.xSmall
            font.letterSpacing: Theme.drum.labelSpacing
        }

        WeekActivityStrip {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Theme.spacing.large
            activity: WorkoutHistoryViewModel.weekActivity
            todayIndex: (new Date().getDay() + 6) % 7
        }

        // Top exercises (most frequent weighted lifts in recent workouts) with best 1RM
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacing.xLarge
            spacing: Theme.stat.spacing

            Repeater {
                model: WorkoutHistoryViewModel.topExercises

                StatTile {
                    objectName: "topExerciseTile_" + modelData.name
                    Layout.fillWidth: true
                    label: modelData.name
                    value: Math.round(modelData.oneRepMax)
                    unit: "kg"
                    badge: "PR"
                }
            }
        }

        // Time held and ground covered in the same recent workouts
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: WorkoutHistoryViewModel.recentTotals.length > 0
                              ? Theme.stat.spacing : 0
            spacing: Theme.stat.spacing

            Repeater {
                model: WorkoutHistoryViewModel.recentTotals

                StatTile {
                    objectName: "totalsTile_" + modelData.label
                    Layout.fillWidth: true
                    label: modelData.label
                    value: modelData.value
                }
            }
        }

        WorkoutDrum {
            objectName: "workoutDrum"
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacing.xLarge
            entries: root.drumEntries
            defaultIndex: root.drumEntries.length > 1 ? root.drumEntries.length - 2 : 0
        }

        ThemedButton {
            objectName: "startWorkoutButton"
            Layout.fillWidth: true
            Layout.leftMargin: Theme.drum.cardInset
            Layout.rightMargin: Theme.drum.cardInset
            Layout.topMargin: Theme.spacing.xLarge
            text: "Start workout"
            iconSource: Theme.icons.startWorkout
            pill: true
            buttonStyle: Theme.button.primary
            buttonSize: Theme.button.wide
            onClicked: {
                if (ActiveWorkoutViewModel.currentWorkout)
                    startWorkoutPopup.open()
                else if (!PlannedWorkoutViewModel.nextWorkout)
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
        if (!PlannedWorkoutViewModel.nextWorkout)
            return
        ActiveWorkoutViewModel.startWorkout(PlannedWorkoutViewModel.nextWorkout)
        PlannedWorkoutViewModel.loadAll()
        if (stackView.currentItem !== activeWorkoutScreen) {
            stackView.replace(activeWorkoutScreen)
        }
    }
}
