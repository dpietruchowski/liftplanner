import QtQuick
import QtQuick.Controls
import LiftPlanner
import Themed.Components

Item {
    id: root
    anchors.fill: parent

    property var activeWorkoutScreen: ScreenActiveWorkout {}
    property var homeScreen: ScreenHome {}
    property var workoutsScreen: ScreenWorkouts {
        onCreateWorkoutRequest: root.openWorkoutEditor()
        onOpenTemplatesRequest: {
            workoutTemplatesScreen.reset()
            stackView.push(workoutTemplatesScreen)
        }
    }
    property var profileScreen: ScreenProfile {}

    property var workoutEditorScreen: ScreenWorkoutEditor {
        onClosed: stackView.pop()
        onAddExerciseRequested: {
            exercisePickerScreen.reset()
            stackView.push(exercisePickerScreen)
        }
    }

    property var exercisePickerScreen: ScreenExercisePicker {
        onCancelled: stackView.pop()
        onExerciseSelected: function(definition) {
            WorkoutEditorViewModel.addExercise(definition)
            stackView.pop()
        }
    }

    property var workoutTemplatesScreen: ScreenWorkoutTemplates {
        onClosed: stackView.pop()
        onTemplateChosen: function(templateId) {
            WorkoutEditorViewModel.startFromTemplate(templateId, new Date())
            stackView.pop()
            stackView.push(workoutEditorScreen)
        }
    }

    function openWorkoutEditor() {
        WorkoutEditorViewModel.createNew("", new Date())
        stackView.push(workoutEditorScreen)
    }

    function goBack() {
        if (stackView.depth > 1) {
            stackView.pop()
            return true
        }
        return false
    }

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

    Connections {
        target: WorkoutEditorViewModel

        function onErrorOccurred(error) {
            notificationPopup.type = Notification.Type.Error
            notificationPopup.text = error
            notificationPopup.open()
        }

        function onSaved(workoutId) {
            PlannedWorkoutViewModel.loadAll()
            while (stackView.currentItem !== workoutsScreen && stackView.depth > 1)
                stackView.pop()
        }

        function onSavedAsTemplate(templateId) {
            notificationPopup.type = Notification.Type.Info
            notificationPopup.text = "Saved as a template. You can start from it via the templates button on the Workouts screen."
            notificationPopup.open()
        }
    }

    Connections {
        target: WorkoutTemplateViewModel

        function onErrorOccurred(error) {
            notificationPopup.type = Notification.Type.Error
            notificationPopup.text = error
            notificationPopup.open()
        }
    }

    ThemedBottomNavigation {
        id: bottomNav
        objectName: "bottomNav"
        anchors.bottom: parent.bottom
        width: parent.width
        model: [
            { name: "navWorkoutButton", label: "Workout", screen: activeWorkoutScreen, icon: Theme.icons.barbell },
            { name: "navHomeButton", label: "Home", screen: homeScreen, icon: Theme.icons.home },
            { name: "navWorkoutsButton", label: "Workouts", screen: workoutsScreen, icon: Theme.icons.calendar },
            { name: "navProfileButton", label: "Profile", screen: profileScreen, icon: Theme.icons.user }
        ]
    }
}
