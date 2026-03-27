import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    color: Theme.colors.background

    anchors.margins: Theme.padding.medium

    property var lastWorkoutName: WorkoutHistoryService.lastWorkout ?
                                      WorkoutHistoryService.lastWorkout.name : "---"
    property var nextWorkoutName: PlannedWorkoutService.nextWorkout ?
                                      PlannedWorkoutService.nextWorkout.name : "---"

    Loader {
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        active: PlannedWorkoutService.workouts.length === 0
        sourceComponent: noPlannedInstruction
    }

    Item {
        id: wrapper
        width: parent.width
        height: columnLayout.implicitHeight
        anchors.verticalCenter: parent.verticalCenter

        ColumnLayout {
            id: columnLayout
            width: parent.width - 2 * Theme.padding.medium
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Theme.spacing.medium
            visible: PlannedWorkoutService.workouts.length > 0

            // Date
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: Qt.formatDate(new Date(), "dddd, d MMMM yyyy")
                color: Theme.colors.textSecondary
                font.pixelSize: Theme.fontSize.small
            }

            // Stat tiles
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: Theme.spacing.medium

                Rectangle {
                    width: 120
                    height: 64
                    radius: Theme.radius.medium
                    color: Theme.colors.surface

                    Column {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: PlannedWorkoutService.workouts.length
                            color: Theme.colors.textPrimary
                            font.pixelSize: Theme.fontSize.xlarge
                            font.bold: true
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Planned"
                            color: Theme.colors.textSecondary
                            font.pixelSize: Theme.fontSize.small
                        }
                    }
                }

                Rectangle {
                    width: 120
                    height: 64
                    radius: Theme.radius.medium
                    color: Theme.colors.surface

                    Column {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: WorkoutHistoryService.workouts.length
                            color: Theme.colors.textPrimary
                            font.pixelSize: Theme.fontSize.xlarge
                            font.bold: true
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Done"
                            color: Theme.colors.textSecondary
                            font.pixelSize: Theme.fontSize.small
                        }
                    }
                }
            }

            // Next planned (smaller)
            Column {
                Layout.alignment: Qt.AlignHCenter
                spacing: 2

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "planned workout"
                    color: Theme.colors.textSecondary
                    font.pixelSize: Theme.fontSize.small
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.spacing.small

                    Text {
                        text: root.nextWorkoutName
                        color: Theme.colors.textSecondary
                        font.pixelSize: Theme.fontSize.medium
                        font.bold: true
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: PlannedWorkoutService.nextWorkout
                              ? Qt.formatDate(PlannedWorkoutService.nextWorkout.plannedTime, "d MMM")
                              : ""
                        color: Theme.colors.textSecondary
                        font.pixelSize: Theme.fontSize.small
                    }
                }
            }

            // Current workout (large, prominent)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.layout.cardHeight * 1.4
                radius: Theme.radius.large
                color: Theme.colors.surface
                border.width: Theme.border.medium
                border.color: Theme.colors.primaryVariant

                Column {
                    anchors.centerIn: parent
                    spacing: Theme.spacing.small

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "planned workout"
                        color: Theme.colors.textSecondary
                        font.pixelSize: Theme.fontSize.small
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: Theme.spacing.small

                        Text {
                            text: root.nextWorkoutName
                            color: Theme.colors.textPrimary
                            font.pixelSize: Theme.fontSize.xlarge
                            font.bold: true
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: PlannedWorkoutService.nextWorkout
                                  ? Qt.formatDate(PlannedWorkoutService.nextWorkout.plannedTime, "d MMM")
                                  : ""
                            color: Theme.colors.textSecondary
                            font.pixelSize: Theme.fontSize.medium
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (ActiveWorkoutService.currentWorkout) {
                            startWorkoutPopup.open()
                        } else {
                            startWorkout()
                        }
                    }
                }
            }

            // Last workout (smaller)
            Column {
                Layout.alignment: Qt.AlignHCenter
                spacing: 2

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "last workout"
                    color: Theme.colors.textSecondary
                    font.pixelSize: Theme.fontSize.small
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.spacing.small

                    Text {
                        text: root.lastWorkoutName
                        color: Theme.colors.textSecondary
                        font.pixelSize: Theme.fontSize.medium
                        font.bold: true
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: WorkoutHistoryService.lastWorkout
                              ? Qt.formatDate(WorkoutHistoryService.lastWorkout.endedTime, "d MMM")
                              : ""
                        color: Theme.colors.textSecondary
                        font.pixelSize: Theme.fontSize.small
                    }
                }
            }
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

    function startWorkout() {
        if (!PlannedWorkoutService.nextWorkout)
            return
        ActiveWorkoutService.startWorkout(PlannedWorkoutService.nextWorkout)
        PlannedWorkoutService.loadAll()
        if (stackView.currentItem !== activeWorkoutScreen) {
            stackView.replace(activeWorkoutScreen)
        }
    }

    Component {
        id: noPlannedInstruction

        Rectangle {
            radius: Theme.radius.medium
            color: Theme.colors.surface

            Text {
                anchors.fill: parent
                text: "You don't have any planned workouts yet.\n\n" +
                      "Go to the 'Planned' tab and click 'Prompt'. " +
                      "The prompt will be copied to your clipboard. " +
                      "Paste it into any AI (ChatGPT, Gemini, etc.) and discuss your training goals. " +
                      "Once the AI generates workouts in JSON format, copy the JSON and return to this app. " +
                      "In the 'Planned' tab, click 'Import' to add your workouts."
                color: Theme.colors.textPrimary
                font.pixelSize: Theme.fontSize.medium
                wrapMode: Text.WordWrap
            }
        }
    }
}
