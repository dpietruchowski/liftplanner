import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LiftPlanner 1.0

Rectangle {
    id: root
    width: 360
    height: 640
    color: Theme.background

    property var lastWorkoutName: WorkoutHistoryService.lastWorkout ?
                                      WorkoutHistoryService.lastWorkout.name : "---"
    property var nextWorkoutName: RoutineService.nextWorkout ?
                                      RoutineService.nextWorkout.name : "---"

    Connections {
        target: RoutineService
        function onNextWorkoutChanged() {
            console.log("next workout", RoutineService.nextWorkout)
        }
    }

    Loader {
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        active: RoutineService.workouts.length === 0
        sourceComponent: noRoutinesInstruction
    }

    ColumnLayout {
        visible: RoutineService.workouts.length > 0
        width: parent.width - 2 * Theme.padding   // cała szerokość z marginesami
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.spacing

        Rectangle {
            width: parent.width
            height: 100
            radius: Theme.borderRadius
            color: Theme.surface

            Column {
                anchors.fill: parent
                anchors.margins: Theme.padding
                spacing: 4

                Text {
                    text: "Your last workout"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontMedium
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.lastWorkoutName
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontLarge
                    font.bold: true
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 100
            radius: Theme.borderRadius
            color: Theme.surface

            Column {
                anchors.fill: parent
                anchors.margins: Theme.padding
                spacing: 4

                Text {
                    text: "Next"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontMedium
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.nextWorkoutName
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontLarge
                    font.bold: true
                }
            }
        }

        PrimaryButton {
            Layout.alignment: Qt.AlignHCenter
            text: "Start workout"
            buttonTheme: Theme.buttonLarge
            buttonStyle: Theme.buttonStylePrimary
            onClicked: {
                if (ActiveWorkoutService.currentWorkout) {
                     startWorkoutPopup.open()
                 } else {
                     startWorkout()
                 }
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
        if (!RoutineService.nextWorkout)
            return
        ActiveWorkoutService.startWorkout(RoutineService.nextWorkout)
        if (stackView.currentItem !== activeWorkoutScreen) {
            stackView.replace(activeWorkoutScreen)
        }
    }

    Component {
        id: noRoutinesInstruction

        Rectangle {
            radius: Theme.borderRadius
            color: Theme.surface

            Text {
                anchors.fill: parent
                text: "You don’t have any routines yet.\n\n" +
                      "Go to the 'Routines' tab and click the 'Prompt' button. " +
                      "The prompt for the language model will be copied to your clipboard. " +
                      "Paste it into any AI (ChatGPT, Gemini, etc.) and discuss your training goals. " +
                      "Once the AI generates a workout in JSON format, copy it to your clipboard and return to this app. " +
                      "In the 'Routines' tab, click the 'Import' button. Your routines should now be imported."
                color: Theme.textPrimary
                font.pixelSize: Theme.fontMedium
                wrapMode: Text.WordWrap
            }
        }
    }
}
