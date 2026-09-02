import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components
import App.Components

Rectangle {
    id: root
    objectName: "screenWorkoutEditor"
    color: Theme.colors.background

    signal closed()
    signal addExerciseRequested()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding.screen
        spacing: Theme.spacing.medium

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.medium

            ThemedButton {
                objectName: "editorBackButton"
                iconSource: Theme.icons.back
                buttonSize: Theme.button.smallSquare
                buttonStyle: Theme.button.subtle
                onClicked: root.closed()
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("New workout")
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.large
            }

            ThemedButton {
                objectName: "saveWorkoutButton"
                text: qsTr("Save")
                buttonSize: Theme.button.small
                buttonStyle: Theme.button.primary
                enabled: WorkoutEditorViewModel.valid && WorkoutEditorViewModel.dirty
                onClicked: WorkoutEditorViewModel.save()
            }
        }

        ThemedInput {
            id: nameField
            objectName: "workoutNameField"
            Layout.fillWidth: true
            placeholder: qsTr("Workout name")
            horizontalAlignment: Text.AlignLeft
            leftPadding: Theme.padding.large
            text: WorkoutEditorViewModel.name
            onTextChanged: WorkoutEditorViewModel.name = text
        }

        ScrollView {
            id: scrollView
            objectName: "editorScrollView"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout {
                width: scrollView.contentItem.width
                spacing: Theme.spacing.medium

                Repeater {
                    id: exerciseRepeater
                    model: WorkoutEditorViewModel.workout
                        ? WorkoutEditorViewModel.workout.exercises
                        : []

                    delegate: EditorExerciseItem {
                        objectName: "editorExerciseItem" + index
                        Layout.fillWidth: true
                        exercise: modelData
                        exerciseIndex: index
                        first: index === 0
                        last: index === exerciseRepeater.count - 1
                    }
                }

                Text {
                    objectName: "editorEmptyHint"
                    Layout.fillWidth: true
                    visible: exerciseRepeater.count === 0
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("No exercises yet.\nTap the button below to pick one from the catalog.")
                    color: Theme.colors.textMuted
                    font.pixelSize: Theme.fontSize.small
                }

                ThemedButton {
                    objectName: "addExerciseButton"
                    Layout.fillWidth: true
                    text: qsTr("Add exercise")
                    iconSource: Theme.icons.plus
                    buttonStyle: Theme.button.tonal
                    onClicked: root.addExerciseRequested()
                }

                ThemedButton {
                    objectName: "saveAsTemplateButton"
                    Layout.fillWidth: true
                    visible: WorkoutEditorViewModel.valid
                    text: qsTr("Save as template")
                    buttonStyle: Theme.button.subtle
                    onClicked: WorkoutEditorViewModel.saveAsTemplate(WorkoutEditorViewModel.name)
                }
            }
        }
    }
}
