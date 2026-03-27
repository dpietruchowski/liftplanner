import QtQuick
import QtQuick.Controls
import LiftPlanner
import Themed.Components

Item {
    id: root
    anchors.fill: parent

    property var activeWorkoutScreen: ScreenActiveWorkout {}
    property var homeScreen: ScreenHome {}
    property var routinesScreen: ScreenRoutines {}
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
        bottomNav.currentIndex = 1
    }

    NotificationPopup {
        id: notificationPopup
        copyEnabled: true
        type: Notification.Type.Error
        buttons: Notification.Button.Ok
    }

    Connections {
        target: RoutineService
        function onErrorOccurred(error) {
            notificationPopup.type = Notification.Type.Error
            notificationPopup.text = error
            notificationPopup.open()
        }

        function onGptPromptGenerated() {
            notificationPopup.type = Notification.Type.Info
            notificationPopup.text =
                "Prompt copied to clipboard.\n\n" +
                "Use this prompt in an AI (ChatGPT, Gemini, etc.) to generate a workout in JSON. " +
                "Then copy the generated JSON and click the 'Import' button in routines to add it."
            notificationPopup.open()
        }
    }

    BottomNavigation {
        id: bottomNav
        anchors.bottom: parent.bottom
        width: parent.width
        model: [
            { label: "Workout", screen: activeWorkoutScreen, icon: Theme.icons.barbell },
            { label: "Home", screen: homeScreen, icon: Theme.icons.home },
            { label: "Routines", screen: routinesScreen, icon: Theme.icons.routines },
            { label: "History", screen: historyScreen, icon: Theme.icons.calendar }
        ]
    }
}
