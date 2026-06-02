import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    color: Theme.colors.background

    RestDialog {
        id: restDialog
        dialogVisible: false
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding.medium
        anchors.topMargin: restDialog.isVisible
                           ? restDialog.barHeight + Theme.padding.medium
                           : Theme.padding.medium
        spacing: Theme.spacing.medium

        Behavior on anchors.topMargin { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }

        Text {
            text: ActiveWorkoutService.currentWorkout ? ActiveWorkoutService.currentWorkout.name : "Workout"
            color: Theme.colors.textPrimary
            font.pixelSize: Theme.fontSize.large
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        ScrollView {
            id: sv
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Column {
                id: contentColumn
                width: sv.availableWidth
                spacing: Theme.spacing.medium / 2

                Repeater {
                    model: ActiveWorkoutService.currentWorkout ? ActiveWorkoutService.currentWorkout.exercises : []

                    delegate: ActiveWorkoutExerciseItem {
                        exercise: modelData
                        onShowExerciseInfo: function(exercise) {
                            exerciseInfoPopup.title = exercise.name
                            var searchUrl = "https://www.youtube.com/results?search_query="
                                    + encodeURIComponent(exercise.name)
                            exerciseInfoPopup.text = exercise.description
                                    + "<br><br><a href=\""
                                    + searchUrl
                                    + "\">YouTube</a>"
                            exerciseInfoPopup.open()
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spacing.medium

            ThemedButton {
                iconSource: Theme.icons.previous
                enabled: ActiveWorkoutService.isActive
                buttonSize: Theme.button.medium
                buttonStyle: Theme.button.primary
                onClicked: ActiveWorkoutService.navigateToPrevious()
            }

            ThemedButton {
                text: ActiveWorkoutService.currentWorkout && ActiveWorkoutService.currentWorkout.completed ? "End" : "Done"
                enabled: ActiveWorkoutService.isActive && ActiveWorkoutService.currentSet
                buttonSize: Theme.button.medium
                buttonStyle: ActiveWorkoutService.currentWorkout && ActiveWorkoutService.currentWorkout.completed
                             ? Theme.button.primary
                             : Theme.button.success
                onClicked: {
                    if (ActiveWorkoutService.currentWorkout.completed) {
                        endWorkoutPopup.open()
                    } else {
                        ActiveWorkoutService.completeCurrentSet()
                        var rs = ActiveWorkoutService.currentExercise ? ActiveWorkoutService.currentExercise.restSeconds : 0
                        if (rs > 0) {
                            restDialog.show(rs)
                        }
                    }
                }
            }

            ThemedButton {
                iconSource: Theme.icons.next
                enabled: ActiveWorkoutService.isActive
                buttonSize: Theme.button.medium
                buttonStyle: Theme.button.primary
                onClicked: ActiveWorkoutService.navigateToNext()
            }
        }
    }

    NotificationPopup {
        id: exerciseInfoPopup
        iconVisible: false
        buttons: Notification.Button.Ok
        textFormat: Text.RichText
    }

    NotificationPopup {
        id: endWorkoutPopup
        text: "Do you want to end workout?"
        type: Notification.Type.Info
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            ActiveWorkoutService.endWorkout()
        }
    }

    Connections {
        target: ActiveWorkoutService
        function onWorkoutCompleted() {
            WorkoutHistoryService.saveWorkout(ActiveWorkoutService.currentWorkout)
            stackView.pop()
        }
    }
}
