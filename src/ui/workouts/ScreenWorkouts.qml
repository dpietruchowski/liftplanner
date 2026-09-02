import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.impl
import LiftPlanner 1.0
import Themed.Components
import App.Components

Rectangle {
    id: root
    objectName: "screenWorkouts"
    color: Theme.colors.background

    property var currentWorkout
    property var workoutToDelete

    signal createWorkoutRequest()

    property var historyMonths: {
        var groups = []
        var workouts = WorkoutHistoryViewModel.workouts
        var current = null
        for (var i = 0; i < workouts.length; ++i) {
            var key = Qt.formatDateTime(workouts[i].startedTime, "yyyy-MM")
            if (!current || current.key !== key) {
                current = {
                    key: key,
                    label: Qt.formatDateTime(workouts[i].startedTime, "MMMM yyyy"),
                    workouts: []
                }
                groups.push(current)
            }
            current.workouts.push(workouts[i])
        }
        return groups
    }

    Component.onCompleted: PlannedWorkoutViewModel.loadAll()

    ScrollView {
        id: scrollView
        objectName: "workoutsScrollView"
        anchors.fill: parent
        anchors.margins: Theme.padding.screen

        ColumnLayout {
            width: scrollView.contentItem.width
            spacing: Theme.spacing.medium

            SectionHeader {
                id: plannedSection
                Layout.fillWidth: true
                title: "PLANNED"
                titleColor: Theme.colors.textMuted
                expandable: true

                ThemedButton {
                    objectName: "createWorkoutButton"
                    iconSource: Theme.icons.plus
                    buttonSize: Theme.button.smallSquare
                    buttonStyle: Theme.button.tonal
                    onClicked: root.createWorkoutRequest()
                    ToolTip.visible: hovered
                    ToolTip.text: "Create a workout"
                    ToolTip.delay: 500
                }

                ThemedButton {
                    objectName: "generatePromptButton"
                    iconSource: Theme.icons.ai
                    buttonSize: Theme.button.smallSquare
                    buttonStyle: Theme.button.tonal
                    onClicked: PlannedWorkoutViewModel.generatePrompt()
                    ToolTip.visible: hovered
                    ToolTip.text: "Generate prompt for AI"
                    ToolTip.delay: 500
                }

                ThemedButton {
                    objectName: "importPlannedButton"
                    iconSource: Theme.icons.importData
                    buttonSize: Theme.button.smallSquare
                    buttonStyle: Theme.button.subtle
                    onClicked: importPlannedPopup.open()
                    ToolTip.visible: hovered
                    ToolTip.text: "Import planned workouts"
                    ToolTip.delay: 500
                }
            }

            RevealColumn {
                expanded: plannedSection.expanded

                Repeater {
                    id: plannedRepeater
                    model: PlannedWorkoutViewModel.workouts
                    delegate: PlannedWorkoutItem {
                        workout: modelData
                        onStartWorkoutRequest: function(workout) {
                            root.currentWorkout = workout
                            if (ActiveWorkoutViewModel.currentWorkout) {
                                startWorkoutPopup.open()
                            } else {
                                startWorkout()
                            }
                        }
                    }
                }
            }

            Text {
                visible: plannedSection.expanded && plannedRepeater.count === 0
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: "No planned workouts.\nTap the AI button to generate a plan prompt."
                font.pixelSize: Theme.fontSize.small
                color: Theme.colors.textMuted
            }

            SectionHeader {
                id: historySection
                Layout.fillWidth: true
                Layout.topMargin: Theme.spacing.large
                title: "HISTORY"
                titleColor: Theme.colors.textMuted
                expandable: true

                ThemedButton {
                    objectName: "exportHistoryButton"
                    iconSource: Theme.icons.exportData
                    buttonSize: Theme.button.smallSquare
                    buttonStyle: Theme.button.subtle
                    onClicked: WorkoutHistoryViewModel.exportToClipboard(50)
                    ToolTip.visible: hovered
                    ToolTip.text: "Export recent workouts"
                    ToolTip.delay: 500
                }

                ThemedButton {
                    objectName: "importHistoryButton"
                    iconSource: Theme.icons.importData
                    buttonSize: Theme.button.smallSquare
                    buttonStyle: Theme.button.subtle
                    onClicked: importHistoryPopup.open()
                    ToolTip.visible: hovered
                    ToolTip.text: "Import workout history"
                    ToolTip.delay: 500
                }
            }

            RevealColumn {
                expanded: historySection.expanded
                spacing: Theme.spacing.large

                Repeater {
                    model: root.historyMonths
                    delegate: HistoryMonthSection {
                        Layout.fillWidth: true
                        monthLabel: modelData.label
                        workouts: modelData.workouts
                        expanded: index === 0

                        onDeleteWorkoutRequest: function(workout) {
                            root.workoutToDelete = workout
                            deletePopup.open()
                        }
                        onExportWorkoutRequest: function(workout) {
                            WorkoutHistoryViewModel.exportWorkoutToClipboard(workout)
                        }
                    }
                }
            }

            Text {
                visible: historySection.expanded && root.historyMonths.length === 0
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: "No completed workouts yet."
                font.pixelSize: Theme.fontSize.small
                color: Theme.colors.textMuted
            }
        }
    }

    function startWorkout() {
        if (!currentWorkout)
            return
        ActiveWorkoutViewModel.startWorkout(currentWorkout)
        PlannedWorkoutViewModel.loadAll()
        if (stackView.currentItem !== activeWorkoutScreen) {
            stackView.replace(activeWorkoutScreen)
        }
    }

    NotificationPopup {
        id: startWorkoutPopup
        text: "Previous workout was not ended. Do you want to start new one?"
        type: Notification.Type.Warning
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            startWorkout()
        }
    }

    NotificationPopup {
        id: importPlannedPopup
        text: "Do you want to import new planned workouts?"
        type: Notification.Type.Warning
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            PlannedWorkoutViewModel.importFromClipboard()
        }
    }

    NotificationPopup {
        id: importHistoryPopup
        text: "Import workout history from clipboard?"
        type: Notification.Type.Info
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            WorkoutHistoryViewModel.importFromClipboard()
        }
    }

    NotificationPopup {
        id: deletePopup
        text: "Do you want to delete workout?"
        type: Notification.Type.Info
        buttons: Notification.Button.Ok | Notification.Button.Cancel
        onAccepted: {
            if (root.workoutToDelete) {
                WorkoutHistoryViewModel.deleteWorkout(root.workoutToDelete)
                root.workoutToDelete = null
            }
        }
    }

    NotificationPopup {
        id: exportedPopup
        text: "Copied to clipboard!"
        type: Notification.Type.Info
        buttons: Notification.Button.Ok
    }

    Connections {
        target: WorkoutHistoryViewModel
        function onExportedToClipboard() { exportedPopup.open() }
    }
}
