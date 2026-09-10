import QtQuick
import QtQuick.Controls
import LiftPlanner
import Themed.Components

Item {
    id: root
    objectName: "mainView"
    anchors.fill: parent

    property var activeWorkoutScreen: ScreenActiveWorkout {}
    property var homeScreen: ScreenHome {}
    property var workoutsScreen: ScreenWorkouts {
        onCreateWorkoutRequest: root.openWorkoutEditor()
        onEditWorkoutRequest: function(workoutId) { root.editPlannedWorkout(workoutId) }
        onOpenTemplatesRequest: {
            workoutTemplatesScreen.reset()
            stackView.push(workoutTemplatesScreen)
        }
    }
    property var profileScreen: ScreenProfile {
        onOpenLicensesRequest: stackView.push(licensesScreen)
    }

    property var licensesScreen: ScreenLicenses {
        onClosed: stackView.pop()
    }

    property var workoutEditorScreen: ScreenWorkoutEditor {
        onClosed: root.closeWorkoutEditor()
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

    function editPlannedWorkout(workoutId) {
        WorkoutEditorViewModel.edit(workoutId)
        stackView.push(workoutEditorScreen)
    }

    function closeWorkoutEditor() {
        if (WorkoutEditorViewModel.dirty) {
            discardChangesDialog.open()
            return
        }
        WorkoutEditorViewModel.discard()
        stackView.pop()
    }

    function goBack() {
        if (discardChangesDialog.opened)
            return true
        if (stackView.currentItem === workoutEditorScreen) {
            closeWorkoutEditor()
            return true
        }
        if (stackView.depth > 1) {
            stackView.pop()
            return true
        }
        return false
    }

    ThemedDialog {
        id: discardChangesDialog
        objectName: "discardChangesDialog"
        dialogTitle: qsTr("Discard changes?")
        message: qsTr("This workout has unsaved changes. Leaving now will throw them away.")
        dialogType: "error"
        acceptText: qsTr("Discard")
        rejectText: qsTr("Keep editing")
        showRejectButton: true
        onAccepted: {
            WorkoutEditorViewModel.discard()
            stackView.pop()
        }
    }

    StackView {
        id: stackView
        objectName: "mainStack"
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
        objectName: "notificationPopup"
        type: Notification.Type.Error
        buttons: Notification.Button.Ok
    }

    function showError(error) {
        notificationPopup.type = Notification.Type.Error
        notificationPopup.title = qsTr("Something went wrong")
        notificationPopup.text = error
        notificationPopup.copyEnabled = true
        notificationPopup.open()
    }

    function showInfo(title, text) {
        notificationPopup.type = Notification.Type.Info
        notificationPopup.title = title
        notificationPopup.text = text
        notificationPopup.copyEnabled = false
        notificationPopup.open()
    }

    Connections {
        target: PlannedWorkoutViewModel
        function onErrorOccurred(error) { root.showError(error) }

        function onPromptGenerated() {
            root.showInfo(qsTr("Prompt copied"),
                          qsTr("Paste it into any AI (ChatGPT, Gemini, etc.) and discuss your training plan. " +
                               "Then copy the generated JSON and tap the import button next to 'Planned' to add planned workouts."))
        }
    }

    Connections {
        target: WorkoutEditorViewModel

        function onErrorOccurred(error) { root.showError(error) }

        function onSaved(workoutId) {
            PlannedWorkoutViewModel.loadAll()
            while (stackView.currentItem !== workoutsScreen && stackView.depth > 1)
                stackView.pop()
        }

        function onSavedAsTemplate(templateId) {
            root.showInfo(qsTr("Saved as a template"),
                          qsTr("You can start from it via the templates button on the Workouts screen."))
        }
    }

    Connections {
        target: WorkoutTemplateViewModel
        function onErrorOccurred(error) { root.showError(error) }
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
