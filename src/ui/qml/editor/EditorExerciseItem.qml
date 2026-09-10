import QtQuick
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root

    property var exercise
    property int exerciseIndex: 0
    property bool first: false
    property bool last: false

    implicitHeight: content.implicitHeight + Theme.padding.medium * 2
    radius: Theme.radius.medium
    color: Theme.colors.cardBackground
    border.width: Theme.border.thin
    border.color: Theme.colors.cardBorder

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: Theme.padding.medium
        spacing: Theme.spacing.small

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.small

            Text {
                Layout.fillWidth: true
                text: root.exercise ? root.exercise.name : ""
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.medium
                elide: Text.ElideRight
            }

            ThemedButton {
                objectName: "moveExerciseUpButton"
                iconSource: Theme.icons.moveUp
                buttonSize: Theme.button.smallSquare
                buttonStyle: Theme.button.subtle
                enabled: !root.first
                onClicked: WorkoutEditorViewModel.moveExercise(root.exerciseIndex,
                                                               root.exerciseIndex - 1)
            }

            ThemedButton {
                objectName: "moveExerciseDownButton"
                iconSource: Theme.icons.moveDown
                buttonSize: Theme.button.smallSquare
                buttonStyle: Theme.button.subtle
                enabled: !root.last
                onClicked: WorkoutEditorViewModel.moveExercise(root.exerciseIndex,
                                                               root.exerciseIndex + 1)
            }

            ThemedButton {
                objectName: "removeExerciseButton"
                iconSource: Theme.icons.removeSet
                buttonSize: Theme.button.smallSquare
                buttonStyle: Theme.button.dangerSubtle
                onClicked: WorkoutEditorViewModel.removeExercise(root.exerciseIndex)
            }
        }

        Repeater {
            model: root.exercise ? root.exercise.sets : []

            delegate: SetRow {
                objectName: "editorSetItem" + root.exerciseIndex + "_" + index
                Layout.fillWidth: true
                number: index + 1
                primaryText: modelData.primaryText
                secondaryText: modelData.secondaryText
                metric: modelData.metric
                loadType: modelData.loadType
                secondaryAdjustable: modelData.secondaryAdjustable
                completable: false
                expanded: true
                expandable: false

                onPrimaryAdjusted: function(direction) {
                    WorkoutEditorViewModel.adjustSetPrimary(root.exerciseIndex, index, direction)
                }
                onSecondaryAdjusted: function(direction) {
                    WorkoutEditorViewModel.adjustSetSecondary(root.exerciseIndex, index, direction)
                }
                onDuplicateRequested: WorkoutEditorViewModel.duplicateSet(root.exerciseIndex, index)
                onRemoveRequested: WorkoutEditorViewModel.removeSet(root.exerciseIndex, index)
            }
        }

        ThemedButton {
            objectName: "addSetButton"
            Layout.fillWidth: true
            text: qsTr("Add set")
            iconSource: Theme.icons.addSet
            buttonSize: Theme.button.small
            buttonStyle: Theme.button.sunken
            onClicked: WorkoutEditorViewModel.addSet(root.exerciseIndex)
        }
    }
}
