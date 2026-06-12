import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.impl
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
        Layout.preferredHeight: Math.max(Theme.layout.listItemHeightLarge,
                                         contentRow.implicitHeight + 2 * Theme.padding.medium)
        radius: Theme.radius.medium
        color: Theme.colors.surface
        border.color: Theme.colors.primary
        border.width: Theme.border.thin

        RowLayout {
            id: contentRow
            anchors.fill: parent
            spacing: Theme.spacing.medium
            anchors.margins: Theme.padding.medium

            ThemedButton {
                iconSource: expanded ? Theme.icons.collapse : Theme.icons.expand
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.primary
                onClicked: expanded = !expanded
                ToolTip.visible: hovered
                ToolTip.text: expanded ? "Collapse" : "Expand"
                ToolTip.delay: 500
            }

            Column {
                Layout.fillWidth: true

                Text {
                    text: workout ? workout.name : ""
                    font.pixelSize: Theme.fontSize.medium
                    font.bold: true
                    color: Theme.colors.textPrimary
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: workout ? Qt.formatDateTime(workout.plannedTime, "ddd, d MMM yyyy") : ""
                    font.pixelSize: Theme.fontSize.small
                    color: Theme.colors.textSecondary
                    visible: text.length > 0
                }
            }

            ThemedButton {
                iconSource: Theme.icons.startWorkout
                circular: true
                buttonSize: Theme.button.square
                buttonStyle: Theme.button.primary
                onClicked: {
                    startWorkoutRequest(workout)
                }
                ToolTip.visible: hovered
                ToolTip.text: "Start workout"
                ToolTip.delay: 500
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
        visible: expanded
        Layout.fillWidth: true
        height: Theme.border.thin
        color: Theme.colors.border
    }
}
