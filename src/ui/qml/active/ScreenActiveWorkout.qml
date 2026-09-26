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

    signal addExerciseRequested()
    signal startEmptyWorkoutRequested()

    readonly property int exerciseCount: ActiveWorkoutViewModel.currentWorkout
                                         ? ActiveWorkoutViewModel.currentWorkout.exercises.length
                                         : 0

    function leaveActiveWorkout() {
        stackView.replace(homeScreen)
        bottomNav.currentIndex = 1
    }

    function askToFinishWorkout() {
        if (!ActiveWorkoutViewModel.hasAnythingToRecord()) {
            discardWorkoutPopup.title = root.exerciseCount > 0
                                        ? qsTr("Nothing done yet")
                                        : qsTr("Throw this session away?")
            discardWorkoutPopup.text = ActiveWorkoutViewModel.abandonPrompt()
            discardWorkoutPopup.okText = root.exerciseCount > 0
                                         ? qsTr("OK")
                                         : qsTr("Throw away")
            discardWorkoutPopup.open()
            return
        }

        var unticked = ActiveWorkoutViewModel.untickedSetCount()
        endWorkoutPopup.title = unticked > 0 ? qsTr("End and delete unticked sets?") : qsTr("End workout?")
        endWorkoutPopup.type = unticked > 0 ? Notification.Type.Warning : Notification.Type.Info
        endWorkoutPopup.text = ActiveWorkoutViewModel.finishPrompt()
        endWorkoutPopup.open()
    }

    WorkoutTimerBar {
        id: timerBar
        objectName: "workoutTimerBar"
        y: 0
    }

    ThemedButton {
        objectName: "noWorkoutStartButton"
        anchors.centerIn: parent
        visible: !ActiveWorkoutViewModel.isActive
        text: qsTr("Start empty workout")
        iconSource: Theme.icons.startWorkout
        iconSize: Theme.icon.large
        pill: true
        buttonStyle: Theme.button.primary
        buttonSize: Theme.button.large
        onClicked: root.startEmptyWorkoutRequested()
    }

    ColumnLayout {
        objectName: "activeWorkoutContent"
        visible: ActiveWorkoutViewModel.isActive
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
            contentWidth: availableWidth

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

                Item {
                    objectName: "activeWorkoutEmptyState"
                    width: contentColumn.width
                    height: emptyState.implicitHeight + 2 * Theme.spacing.xLarge
                    visible: ActiveWorkoutViewModel.isActive && root.exerciseCount === 0

                    Column {
                        id: emptyState
                        width: parent.width
                        anchors.centerIn: parent
                        spacing: Theme.spacing.medium

                        Text {
                            objectName: "activeWorkoutEmptyTitle"
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            text: qsTr("Add your first exercise")
                            color: Theme.colors.textPrimary
                            font.pixelSize: Theme.fontSize.xMedium
                            font.bold: true
                            wrapMode: Text.WordWrap
                        }

                        Text {
                            objectName: "activeWorkoutEmptyHint"
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            text: qsTr("This session is empty. Pick an exercise and it arrives with a set to tick off and the weight you used last time.")
                            color: Theme.colors.textMuted
                            font.pixelSize: Theme.fontSize.normal
                            wrapMode: Text.WordWrap
                        }

                        ThemedButton {
                            objectName: "addFirstExerciseButton"
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Add exercise")
                            iconSource: Theme.icons.plus
                            pill: true
                            buttonSize: Theme.button.large
                            buttonStyle: Theme.button.primary
                            onClicked: root.addExerciseRequested()
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
                    enabled: ActiveWorkoutViewModel.isActive && root.exerciseCount > 0
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
                    objectName: "addExerciseButton"
                    iconSource: Theme.icons.plus
                    enabled: ActiveWorkoutViewModel.isActive
                    buttonSize: Theme.button.mediumSquare
                    buttonStyle: Theme.button.subtle
                    onClicked: root.addExerciseRequested()
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Add exercise")
                    ToolTip.delay: 500
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
                    visible: root.exerciseCount > 0
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
