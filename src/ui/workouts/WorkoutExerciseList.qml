import QtQuick
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

ColumnLayout {
    id: root

    property var workout
    property bool expanded: false

    visible: expanded
    spacing: Theme.spacing.medium

    Repeater {
        model: root.expanded && root.workout ? root.workout.exercises : []
        delegate: ExerciseItem {
            Layout.fillWidth: true
            exercise: modelData
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: Theme.border.thin
        color: Theme.colors.border
    }
}
