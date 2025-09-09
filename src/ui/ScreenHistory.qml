import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0

Rectangle {
    id: root
    color: Theme.background

    WorkoutListView {
        anchors.fill: parent
        workouts: WorkoutHistoryService.workouts
        workoutDelegate: WorkoutItem {
            workout: modelData
        }
    }
}
