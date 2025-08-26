import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

Rectangle {
    id: root
    color: Theme.background

    Component.onCompleted: WorkoutService.loadAllWorkouts()

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: Theme.padding

        ColumnLayout {
            id: columnLayout
            width: scrollView.contentItem.width
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacing

            WorkoutButtonsRow {}

            Repeater {
                model: WorkoutService.workouts
                delegate: WorkoutItem {
                    workout: modelData
                }
            }
        }
    }
}