import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.impl
import LiftPlanner 1.0
import Themed.Components
import App.Components

Rectangle {
    id: root
    color: Theme.colors.background

    property var currentWorkout
    property var workoutToDelete

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
        anchors.fill: parent
        anchors.margins: Theme.padding.medium

        ColumnLayout {
            width: scrollView.contentItem.width
            spacing: Theme.spacing.medium

            SectionHeader {
                id: plannedSection
                Layout.fillWidth: true
                title: "PLANNED"
                expandable: true

                ThemedButton {
                    iconSource: Theme.icons.ai
                    circular: true
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.primary
                    onClicked: PlannedWorkoutViewModel.generatePrompt()
                    ToolTip.visible: hovered
                    ToolTip.text: "Generate prompt for AI"
                    ToolTip.delay: 500
                }

                ThemedButton {
                    iconSource: Theme.icons.importData
                    circular: true
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.primary
                    onClicked: importPlannedPopup.open()
                    ToolTip.visible: hovered
                    ToolTip.text: "Import planned workouts"
                    ToolTip.delay: 500
                }
            }

            Repeater {
                id: plannedRepeater
                model: plannedSection.expanded ? PlannedWorkoutViewModel.workouts : []
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

            Text {
                visible: plannedSection.expanded && plannedRepeater.count === 0
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: "No planned workouts.\nTap the AI button to generate a plan prompt."
                font.pixelSize: Theme.fontSize.small
                color: Theme.colors.textSecondary
            }

            SectionHeader {
                id: historySection
                Layout.fillWidth: true
                Layout.topMargin: Theme.spacing.large
                title: "HISTORY"
                expandable: true

                ThemedButton {
                    iconSource: Theme.icons.exportData
                    circular: true
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.primary
                    onClicked: WorkoutHistoryViewModel.exportToClipboard(50)
                    ToolTip.visible: hovered
                    ToolTip.text: "Export recent workouts"
                    ToolTip.delay: 500
                }

                ThemedButton {
                    iconSource: Theme.icons.importData
                    circular: true
                    buttonSize: Theme.button.square
                    buttonStyle: Theme.button.primary
                    onClicked: importHistoryPopup.open()
                    ToolTip.visible: hovered
                    ToolTip.text: "Import workout history"
                    ToolTip.delay: 500
                }
            }

            Repeater {
                model: historySection.expanded ? root.historyMonths : []
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

            Text {
                visible: historySection.expanded && root.historyMonths.length === 0
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: "No completed workouts yet."
                font.pixelSize: Theme.fontSize.small
                color: Theme.colors.textSecondary
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
