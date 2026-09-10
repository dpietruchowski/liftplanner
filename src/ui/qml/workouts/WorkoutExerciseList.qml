import QtQuick
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

ColumnLayout {
    id: root

    property var workout
    property bool expanded: false
    property string chipPrefix: ""

    readonly property bool flagsMeaningful: workout ? workout.completionFlagsMeaningful : false

    visible: expanded
    spacing: Theme.spacing.medium

    Repeater {
        model: root.expanded && root.workout ? root.workout.exercises : []
        delegate: ExerciseItem {
            Layout.fillWidth: true
            exercise: modelData
            flagsMeaningful: root.flagsMeaningful
            chipPrefix: root.chipPrefix.length > 0 ? root.chipPrefix + "_" + index : ""
        }
    }
}
