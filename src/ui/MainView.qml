import QtQuick
import QtQuick.Controls
import LiftPlanner
import Themed.Components

Item {
    id: root
    anchors.fill: parent

    property var activeWorkoutScreen: ScreenActiveWorkout {}
    property var homeScreen: ScreenHome {}
    property var plannedScreen: ScreenPlannedWorkouts {}
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
        target: PlannedWorkoutService
        function onErrorOccurred(error) {
            notificationPopup.type = Notification.Type.Error
            notificationPopup.text = error
            notificationPopup.open()
        }

        function onPromptGenerated() {
            notificationPopup.type = Notification.Type.Info
            notificationPopup.text =
                "Prompt copied to clipboard.\n\n" +
                "Paste it into any AI (ChatGPT, Gemini, etc.) and discuss your training plan. " +
                "Then copy the generated JSON and click the 'Import' button to add planned workouts."
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
            { label: "Planned", screen: plannedScreen, icon: Theme.icons.planned },
            { label: "History", screen: historyScreen, icon: Theme.icons.calendar }
        ]
    }
}
