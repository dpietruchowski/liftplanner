import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent

    property int currentIndex: 0

    StackView {
        id: stackView
        anchors.fill: parent
    }

    property var homeScreen: ScreenHome {}
    property var workoutScreen: ScreenWorkout {}
    property var historyScreen: ScreenHistory {}

    Component.onCompleted: {
        stackView.push(homeScreen)
    }

    BottomNavigation {
        id: bottomNav
        anchors.bottom: parent.bottom
        width: parent.width
        onItemSelected: index => {
            currentIndex = index
            switch(index) {
                case 0: stackView.replace(homeScreen); break;
                case 1: stackView.replace(workoutScreen); break;
                case 2: stackView.replace(historyScreen); break;
            }
        }
    }
}