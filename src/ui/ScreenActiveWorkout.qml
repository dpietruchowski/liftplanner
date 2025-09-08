import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LiftPlanner

Rectangle {
    id: root
    color: Theme.background

    property var currentSetter

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding
        spacing: Theme.spacing

        Text {
            text: ActiveWorkoutService.currentWorkout ? ActiveWorkoutService.currentWorkout.name : "Workout"
            color: Theme.textPrimary
            font.pixelSize: Theme.fontLarge
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
                spacing: Theme.spacing / 2

                Repeater {
                    model: ActiveWorkoutService.currentWorkout ? ActiveWorkoutService.currentWorkout.exercises : []

                    delegate: ActiveWorkoutExerciseItem {
                        exercise: modelData
                        onEditSetRepetitions: function(setData) { handleEditRepetitions(setData) }
                        onEditSetWeight: function(setData) { handleEditWeight(setData) }
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spacing

            PrimaryButton {
                svgIcon: Theme.icons.previous
                enabled: ActiveWorkoutService.isActive
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStylePrimary
                onClicked: ActiveWorkoutService.navigateToPrevious()
            }

            PrimaryButton {
                text: ActiveWorkoutService.currentWorkout && ActiveWorkoutService.currentWorkout.completed ? "End" : "Done"
                enabled: ActiveWorkoutService.isActive && ActiveWorkoutService.currentSet
                buttonTheme: Theme.buttonMedium
                buttonStyle: ActiveWorkoutService.currentWorkout && ActiveWorkoutService.currentWorkout.completed
                             ? Theme.buttonStylePrimary
                             : Theme.buttonStyleSuccess
                onClicked: {
                    if (ActiveWorkoutService.currentWorkout.completed) {
                        ActiveWorkoutService.endWorkout()
                    } else {
                        ActiveWorkoutService.completeCurrentSet()
                        var rs = ActiveWorkoutService.currentExercise ? ActiveWorkoutService.currentExercise.restSeconds : 0
                        if (rs > 0) {
                            restDialog.show(rs)
                        }
                    }
                }
            }

            PrimaryButton {
                svgIcon: Theme.icons.next
                enabled: ActiveWorkoutService.isActive
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStylePrimary
                onClicked: ActiveWorkoutService.navigateToNext()
            }
        }

    }

    Item {
        id: overlayRoot
        anchors.fill: parent
        visible: valueEditorDialog.dialogVisible

        Rectangle {
            anchors.fill: parent
            color: "#00000080"

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
