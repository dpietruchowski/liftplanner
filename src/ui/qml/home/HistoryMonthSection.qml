import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner
import Themed.Components
import App.Components

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
        titleFont.pixelSize: Theme.fontSize.normal
        titleFont.letterSpacing: 0
        titleColor: Theme.colors.textSecondary
        expandable: true

        Text {
            text: root.workouts.length
            font.pixelSize: Theme.fontSize.small
            color: Theme.colors.textDisabled
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    RevealColumn {
        expanded: header.expanded

        Repeater {
            model: root.workouts
            delegate: WorkoutItem {
                Layout.fillWidth: true
                workout: modelData
                itemIndex: index
                onDeleteWorkout: function(workout) { root.deleteWorkoutRequest(workout) }
                onExportWorkout: function(workout) { root.exportWorkoutRequest(workout) }
            }
        }
    }
}
