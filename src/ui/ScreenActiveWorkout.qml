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

        SectionHeader {
            id: plannedSection
            Layout.fillWidth: true
            titleFont.pixelSize: Theme.fontSize.large
            title: ActiveWorkoutViewModel.currentWorkout ? ActiveWorkoutViewModel.currentWorkout.name : "Workout"
            visible: !restDialog.isVisible
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
                    model: ActiveWorkoutViewModel.currentWorkout ? ActiveWorkoutViewModel.currentWorkout.exercises : []

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
                enabled: ActiveWorkoutViewModel.isActive
                buttonSize: Theme.button.medium
                buttonStyle: Theme.button.primary
                onClicked: ActiveWorkoutViewModel.navigateToPrevious()
            }

            ThemedButton {
                text: ActiveWorkoutViewModel.currentWorkout && ActiveWorkoutViewModel.currentWorkout.completed ? "End" : "Done"
                enabled: ActiveWorkoutViewModel.isActive && ActiveWorkoutViewModel.currentSet
                buttonSize: Theme.button.medium
                buttonStyle: ActiveWorkoutViewModel.currentWorkout && ActiveWorkoutViewModel.currentWorkout.completed
                             ? Theme.button.primary
                             : Theme.button.success
                onClicked: {
                    if (ActiveWorkoutViewModel.currentWorkout.completed) {
                        endWorkoutPopup.open()
                    } else {
                        ActiveWorkoutViewModel.completeCurrentSet()
                        var rs = ActiveWorkoutViewModel.currentExercise ? ActiveWorkoutViewModel.currentExercise.restSeconds : 0
                        if (rs > 0) {
                            restDialog.show(rs)
                        }
                    }
                }
            }

            ThemedButton {
                iconSource: Theme.icons.next
                enabled: ActiveWorkoutViewModel.isActive
                buttonSize: Theme.button.medium
                buttonStyle: Theme.button.primary
                onClicked: ActiveWorkoutViewModel.navigateToNext()
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
            ActiveWorkoutViewModel.endWorkout()
        }
    }

    Connections {
        target: ActiveWorkoutViewModel
        function onWorkoutCompleted() {
            stackView.replace(homeScreen)
            bottomNav.currentIndex = 1
        }
    }
}
