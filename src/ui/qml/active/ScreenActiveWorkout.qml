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

    function leaveActiveWorkout() {
        stackView.replace(homeScreen)
        bottomNav.currentIndex = 1
    }

    function askToFinishWorkout() {
        var done = ActiveWorkoutViewModel.completedSetCount()
        var total = ActiveWorkoutViewModel.totalSetCount()

        if (!ActiveWorkoutViewModel.hasAnythingToRecord()) {
            discardWorkoutPopup.text = qsTr("No set is ticked off yet, so there is nothing to record. "
                                            + "This workout goes back to your planned list.")
            discardWorkoutPopup.open()
            return
        }

        endWorkoutPopup.text = done === total
            ? qsTr("All %1 sets are ticked off. End this workout?").arg(total)
            : qsTr("%1 of %2 sets are ticked off. The rest stay marked as not done.")
                  .arg(done).arg(total)
        endWorkoutPopup.open()
    }

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
                            root.askToFinishWorkout()
                        } else {
                            ActiveWorkoutViewModel.completeCurrentSet()
                        }
                    }
                }

                ThemedButton {
                    objectName: "finishWorkoutButton"
                    iconSource: Theme.icons.success
                    enabled: ActiveWorkoutViewModel.isActive
                    buttonSize: Theme.button.mediumSquare
                    buttonStyle: Theme.button.tonal
                    onClicked: root.askToFinishWorkout()
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Finish workout")
                    ToolTip.delay: 500
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
        objectName: "endWorkoutPopup"
        title: qsTr("End workout?")
        text: qsTr("Do you want to end workout?")
        type: Notification.Type.Info
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            ActiveWorkoutViewModel.endWorkout()
        }
    }

    NotificationPopup {
        id: discardWorkoutPopup
        objectName: "discardWorkoutPopup"
        title: qsTr("Nothing done yet")
        type: Notification.Type.Warning
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            ActiveWorkoutViewModel.discardWorkout()
        }
    }

    WorkoutSummaryPopup {
        id: workoutSummaryPopup
        objectName: "workoutSummaryPopup"
        summary: ActiveWorkoutViewModel.lastSessionSummary
        onDismissed: root.leaveActiveWorkout()
    }

    Connections {
        target: ActiveWorkoutViewModel

        function onWorkoutCompleted() { workoutSummaryPopup.open() }
        function onWorkoutDiscarded() { root.leaveActiveWorkout() }
    }
}
