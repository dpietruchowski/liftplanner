import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components
import App.Components

Rectangle {
    id: root
    objectName: "screenActiveWorkout"
    color: Theme.colors.background

    WorkoutTimerBar {
        id: timerBar
        objectName: "workoutTimerBar"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding.medium
        anchors.topMargin: timerBar.isVisible
                           ? timerBar.barHeight + Theme.padding.medium
                           : Theme.padding.medium
        spacing: Theme.spacing.medium

        Behavior on anchors.topMargin { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }

        SectionHeader {
            id: plannedSection
            Layout.fillWidth: true
            titleFont.pixelSize: Theme.fontSize.large
            title: ActiveWorkoutViewModel.currentWorkout ? ActiveWorkoutViewModel.currentWorkout.name : "Workout"
            visible: !timerBar.isVisible
        }

        ScrollView {
            id: sv
            objectName: "workoutExerciseList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Column {
                id: contentColumn
                width: sv.availableWidth
                spacing: Theme.spacing.medium / 2

                property var expandedExercise: ActiveWorkoutViewModel.currentExercise
                property bool reorderMode: false

                signal exerciseMoved(var exercise, int direction)

                function requestMove(ex, from, dir) {
                    ActiveWorkoutViewModel.moveExercise(from, from + dir)
                    Qt.callLater(function() { contentColumn.exerciseMoved(ex, dir) })
                }

                Connections {
                    target: ActiveWorkoutViewModel
                    function onCurrentExerciseChanged() {
                        contentColumn.expandedExercise = ActiveWorkoutViewModel.currentExercise
                    }
                }

                Repeater {
                    id: exerciseRepeater
                    model: ActiveWorkoutViewModel.currentWorkout ? ActiveWorkoutViewModel.currentWorkout.exercises : []

                    delegate: ActiveWorkoutExerciseItem {
                        exercise: modelData
                        screen: contentColumn
                        exerciseCount: exerciseRepeater.count
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
            Layout.fillWidth: true
            spacing: Theme.spacing.medium

            ThemedButton {
                objectName: "startTimerButton"
                iconSource: Theme.icons.timer
                enabled: ActiveWorkoutViewModel.isActive
                buttonSize: Theme.button.mediumSquare
                buttonStyle: Theme.button.primary
                onClicked: ActiveWorkoutViewModel.toggleTimer()
            }

            Item { Layout.fillWidth: true }

            ThemedButton {
                objectName: "completeSetButton"
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
                    }
                }
            }

            Item { Layout.fillWidth: true }

            ThemedButton {
                objectName: "reorderButton"
                iconSource: Theme.icons.reorder
                enabled: ActiveWorkoutViewModel.isActive
                buttonSize: Theme.button.mediumSquare
                buttonStyle: contentColumn.reorderMode ? Theme.button.primary : Theme.button.outline
                onClicked: contentColumn.reorderMode = !contentColumn.reorderMode
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
