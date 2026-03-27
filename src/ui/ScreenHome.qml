import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    color: Theme.colors.background

    anchors.margins: Theme.padding.medium


    Loader {
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        active: PlannedWorkoutService.workouts.length === 0
        sourceComponent: noPlannedInstruction
    }

    ColumnLayout {
        width: parent.width - 2 * Theme.padding.medium
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 2
        visible: PlannedWorkoutService.workouts.length > 0

        // Stat tiles
        Row {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: Theme.spacing.large
            spacing: Theme.spacing.medium

            StatTile {
                label: "Done"
                value: WorkoutHistoryService.workouts.length
            }

            StatTile {
                label: "Planned"
                value: PlannedWorkoutService.workouts.length
            }
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
    }

    ThemedButton {
        buttonSize: Theme.button.large
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.padding.medium
        text: "Start workout"
        visible: PlannedWorkoutService.workouts.length > 0

        onClicked: {
            if (ActiveWorkoutService.currentWorkout) {
                startWorkoutPopup.open()
            } else {
                startWorkout()
            }
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

    function startWorkout() {
        if (!PlannedWorkoutService.nextWorkout)
            return
        ActiveWorkoutService.startWorkout(PlannedWorkoutService.nextWorkout)
        PlannedWorkoutService.loadAll()
        if (stackView.currentItem !== activeWorkoutScreen) {
            stackView.replace(activeWorkoutScreen)
        }
    }

    Component {
        id: noPlannedInstruction

        Rectangle {
            radius: Theme.radius.medium
            color: Theme.colors.surface

            Text {
                anchors.fill: parent
                text: "You don't have any planned workouts yet.\n\n" +
                      "Go to the 'Planned' tab and click 'Prompt'. " +
                      "The prompt will be copied to your clipboard. " +
                      "Paste it into any AI (ChatGPT, Gemini, etc.) and discuss your training goals. " +
                      "Once the AI generates workouts in JSON format, copy the JSON and return to this app. " +
                      "In the 'Planned' tab, click 'Import' to add your workouts."
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.medium
                wrapMode: Text.WordWrap
            }
        }
    }
}
