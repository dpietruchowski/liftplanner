import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    color: Theme.colors.background

    anchors.margins: Theme.padding.medium

    property var lastWorkoutName: WorkoutHistoryService.lastWorkout ?
                                      WorkoutHistoryService.lastWorkout.name : "---"
    property var nextWorkoutName: RoutineService.nextWorkout ?
                                      RoutineService.nextWorkout.name : "---"

    onLastWorkoutNameChanged: {
        RoutineService.lastWorkoutName = lastWorkoutName
    }

    Loader {
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        active: RoutineService.workouts.length === 0
        sourceComponent: noRoutinesInstruction
    }
    Item {
        id: wrapper
        width: parent.width
        height: columnLayout.implicitHeight
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Theme.padding.medium
        anchors.rightMargin: Theme.padding.medium

        ColumnLayout {
            id: columnLayout
            width: parent.width - 2 * Theme.padding.medium
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Theme.spacing.medium
            visible: RoutineService.workouts.length > 0

            Rectangle {
                Layout.fillWidth: true
                height: Theme.layout.cardHeight
                radius: Theme.radius.medium
                color: Theme.colors.surface

                Column {
                    anchors.fill: parent
                    anchors.margins: Theme.padding.medium
                    spacing: Theme.spacing.xsmall

                    Text {
                        text: "Your last workout"
                        color: Theme.colors.textSecondary
                        font.pixelSize: Theme.fontSize.medium
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: root.lastWorkoutName
                        color: Theme.colors.textPrimary
                        font.pixelSize: Theme.fontSize.large
                        font.bold: true
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: Theme.layout.cardHeight
                radius: Theme.radius.medium
                color: Theme.colors.surface

                Column {
                    anchors.fill: parent
                    anchors.margins: Theme.padding.medium
                    spacing: Theme.spacing.xsmall

                    Text {
                        text: "Next"
                        color: Theme.colors.textSecondary
                        font.pixelSize: Theme.fontSize.medium
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: root.nextWorkoutName
                        color: Theme.colors.textPrimary
                        font.pixelSize: Theme.fontSize.large
                        font.bold: true
                    }
                }
            }

            ThemedButton {
                Layout.alignment: Qt.AlignHCenter
                text: "Start workout"
                buttonSize: Theme.button.large
                buttonStyle: Theme.button.primary
                onClicked: {
                    if (ActiveWorkoutService.currentWorkout) {
                         startWorkoutPopup.open()
                     } else {
                         startWorkout()
                     }
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
            radius: Theme.radius.medium
            color: Theme.colors.surface

            Text {
                anchors.fill: parent
                text: "You don't have any routines yet.\n\n" +
                      "Go to the 'Routines' tab and click the 'Prompt' button. " +
                      "The prompt for the language model will be copied to your clipboard. " +
                      "Paste it into any AI (ChatGPT, Gemini, etc.) and discuss your training goals. " +
                      "Once the AI generates a workout in JSON format, copy it to your clipboard and return to this app. " +
                      "In the 'Routines' tab, click the 'Import' button. Your routines should now be imported."
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.medium
                wrapMode: Text.WordWrap
            }
        }
    }
}
