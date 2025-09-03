import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

Rectangle {
    id: root
    color: Theme.background

    Component.onCompleted: RoutineService.loadAllWorkouts()

    WorkoutListView {
        anchors.fill: parent
        workouts: RoutineService.workouts
        workoutDelegate: RoutineItem {
            workout: modelData
        }
        buttonsRow: WorkoutButtonsRow {}
    }
}
