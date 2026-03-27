import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    color: Theme.colors.background

    property var currentSetter

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding.medium
        spacing: Theme.spacing.medium

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
                        onEditSetRepetitions: function(setData) { handleEditRepetitions(setData) }
                        onEditSetWeight: function(setData) { handleEditWeight(setData) }
                        onShowExerciseInfo: function(exercise) {
                            exerciseInfoPopup.title = exercise.name
                            exerciseInfoPopup.text = exercise.description
                                    + "<br><br><a href=\""
                                    + exercise.youtubeLink
                                    + "\">"
                                    + exercise.youtubeLink
                                    + "</a>"
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

    Item {
        id: overlayRoot
        anchors.fill: parent
        visible: valueEditorDialog.dialogVisible

        Rectangle {
            anchors.fill: parent
            color: "#000000C0"

            MouseArea {
                anchors.fill: parent
                onClicked: valueEditorDialog.dialogVisible = false
            }
        }

        ValueEditorDialog {
            id: valueEditorDialog
            dialogVisible: false
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            step: 1

            onConfirmed: {
                if (currentSetter) {
                    currentSetter(value)
                }
                currentSetter = null
                dialogVisible = false
            }
        }
    }

    RestDialog {
        id: restDialog
        dialogVisible: false
    }

    Connections {
        target: ActiveWorkoutService
        function onWorkoutCompleted() {
            WorkoutHistoryService.saveWorkout(ActiveWorkoutService.currentWorkout)
            stackView.pop()
        }
    }

    function handleEditRepetitions(setData) {
        currentSetter = function(newValue) { setData.repetitions = newValue }
        valueEditorDialog.value = setData.repetitions
        valueEditorDialog.minValue = 0
        valueEditorDialog.maxValue = 100
        valueEditorDialog.step = 1
        valueEditorDialog.dialogVisible = true
    }

    function handleEditWeight(setData) {
        currentSetter = function(newValue) { setData.weight = newValue }
        valueEditorDialog.value = setData.weight
        valueEditorDialog.minValue = 0
        valueEditorDialog.maxValue = 500
        valueEditorDialog.step = 5
        valueEditorDialog.dialogVisible = true
    }
}
