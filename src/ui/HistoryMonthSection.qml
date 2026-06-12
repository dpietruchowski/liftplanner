import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Themed.Components

ColumnLayout {
    id: root
    spacing: Theme.spacing.medium

    property string monthLabel
    property var workouts: []
    property alias expanded: header.expanded

    signal deleteWorkoutRequest(var workout)
    signal exportWorkoutRequest(var workout)

    SectionHeader {
        id: header
        Layout.fillWidth: true
        title: root.monthLabel
        expandable: true

        Text {
            text: root.workouts.length
            font.pixelSize: Theme.fontSize.small
            color: Theme.colors.textSecondary
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Repeater {
        model: header.expanded ? root.workouts : []
        delegate: WorkoutItem {
            Layout.fillWidth: true
            workout: modelData
            onDeleteWorkout: function(workout) { root.deleteWorkoutRequest(workout) }
            onExportWorkout: function(workout) { root.exportWorkoutRequest(workout) }
        }
    }
}
