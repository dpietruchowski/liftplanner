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

    ColumnLayout {
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
}
