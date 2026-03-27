import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

ColumnLayout {
    Layout.fillWidth: true
    spacing: Theme.spacing.medium
    property var workout
    property bool expanded: false

    signal startWorkoutRequest(var workout)

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 50
        radius: Theme.radius.medium
        color: Theme.colors.surface
        border.color: Theme.colors.primary
        border.width: 1

        RowLayout {
            anchors.fill: parent
            spacing: Theme.spacing.medium
            anchors.margins: Theme.padding.medium

            ThemedButton {
                iconSource: expanded ? Theme.icons.collapse : Theme.icons.expand
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.primary
                onClicked: expanded = !expanded
            }

            Text {
                text: workout.name
                font.pixelSize: Theme.fontSize.medium
                font.bold: true
                color: Theme.colors.textPrimary
                Layout.fillWidth: true
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            ThemedButton {
                iconSource: Theme.icons.startWorkout
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.primary
                onClicked: {
                    startWorkoutRequest(workout)
                }
            }
        }
    }

    Repeater {
        model: expanded ? workout.exercises : []
        delegate: ExerciseItem {
            Layout.fillWidth: true
            exercise: modelData
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: 1
        color: Theme.colors.border
    }
}
