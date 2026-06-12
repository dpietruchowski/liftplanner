import QtQuick
import QtQuick.Controls
import LiftPlanner
import Themed.Components

Item {
    id: root
    anchors.fill: parent

    property var activeWorkoutScreen: ScreenActiveWorkout {}
    property var homeScreen: ScreenHome {}
    property var workoutsScreen: ScreenWorkouts {}
    property var profileScreen: ScreenProfile {}

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
        target: PlannedWorkoutViewModel
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
                "Then copy the generated JSON and tap the import button next to 'Planned' to add planned workouts."
            notificationPopup.open()
        }
    }

    ThemedBottomNavigation {
        id: bottomNav
        anchors.bottom: parent.bottom
        width: parent.width
        model: [
            { label: "Workout", screen: activeWorkoutScreen, icon: Theme.icons.barbell },
            { label: "Home", screen: homeScreen, icon: Theme.icons.home },
            { label: "Workouts", screen: workoutsScreen, icon: Theme.icons.calendar },
            { label: "Profile", screen: profileScreen, icon: Theme.icons.user }
        ]
    }
}
