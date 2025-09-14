import QtQuick 2.15
import QtQuick.Controls 2.15
import LiftPlanner

Item {
    id: root
    anchors.fill: parent

    property var activeWorkoutScreen: ScreenActiveWorkout {}
    property var homeScreen: ScreenHome {}
    property var workoutScreen: ScreenWorkout {}
    property var historyScreen: ScreenHistory {}


    StackView {
        id: stackView
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bottomNav.top
    }

    Component.onCompleted: {
        stackView.push(homeScreen)
    }

    BottomNavigation {
        id: bottomNav
        anchors.bottom: parent.bottom
        width: parent.width
        model: [
            { label: "Workout", screen: activeWorkoutScreen },
            { label: "Home", screen: homeScreen },
            { label: "Routines", screen: workoutScreen },
            { label: "History", screen: historyScreen }
        ]
    }
}
