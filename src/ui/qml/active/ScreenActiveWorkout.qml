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
        y: 0
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: timerBar.isVisible
                           ? timerBar.barHeight + Theme.padding.screen
                           : Theme.padding.screen
        spacing: Theme.spacing.medium

        Behavior on anchors.topMargin { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }

        Text {
            objectName: "activeWorkoutTitle"
            Layout.fillWidth: true
            Layout.leftMargin: Theme.padding.screen
            Layout.rightMargin: Theme.padding.screen
            text: ActiveWorkoutViewModel.currentWorkout ? ActiveWorkoutViewModel.currentWorkout.name : "Workout"
            color: Theme.colors.textPrimary
            font.pixelSize: Theme.fontSize.large
            font.bold: true
            elide: Text.ElideRight
            visible: !timerBar.isVisible
        }

        ScrollView {
            id: sv
            objectName: "workoutExerciseList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: Theme.padding.screen
            Layout.rightMargin: Theme.padding.screen
            clip: true

            Column {
                id: contentColumn
                width: sv.availableWidth
                spacing: Theme.spacing.small

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
                        objectName: "activeWorkoutExerciseItem" + index
                        exercise: modelData
                        exerciseIndex: index
                        screen: contentColumn
                        exerciseCount: exerciseRepeater.count
                        onShowExerciseInfo: function(exercise) {
                            exerciseInfoPanel.showExercise(exercise)
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.layout.actionBarHeight
            color: Theme.colors.surfaceRaised

            Rectangle {
                width: parent.width
                height: Theme.border.thin
                color: Theme.colors.divider
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.padding.screen
                anchors.rightMargin: Theme.padding.screen
                spacing: Theme.spacing.medium

                ThemedButton {
                    objectName: "startTimerButton"
                    iconSource: Theme.icons.timer
                    enabled: ActiveWorkoutViewModel.isActive
                    buttonSize: Theme.button.mediumSquare
                    buttonStyle: Theme.button.tonal
                    onClicked: ActiveWorkoutViewModel.toggleTimer()
                }

                ThemedButton {
                    objectName: "completeSetButton"
                    text: ActiveWorkoutViewModel.currentWorkout && ActiveWorkoutViewModel.currentWorkout.completed ? "End" : "Done"
                    enabled: ActiveWorkoutViewModel.isActive && ActiveWorkoutViewModel.currentSet
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.button.medium.height
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

                ThemedButton {
                    objectName: "reorderButton"
                    iconSource: Theme.icons.reorder
                    enabled: ActiveWorkoutViewModel.isActive
                    buttonSize: Theme.button.mediumSquare
                    buttonStyle: contentColumn.reorderMode ? Theme.button.tonal : Theme.button.subtle
                    onClicked: contentColumn.reorderMode = !contentColumn.reorderMode
                }
            }
        }
    }

    ExerciseInfoPanel {
        id: exerciseInfoPanel
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
