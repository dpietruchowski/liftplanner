import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

Rectangle {
    id: root
    color: Theme.background

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding
        spacing: Theme.spacing

        Text {
            text: ActiveWorkoutService.currentWorkout ? ActiveWorkoutService.currentWorkout.name : "Workout"
            color: Theme.textPrimary
            font.pixelSize: Theme.fontLarge
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            height: 70
            color: Theme.primary
            radius: Theme.borderRadius
            visible: ActiveWorkoutService.currentExercise
            border.color: Theme.primaryVariant
            border.width: 1

            Text {
                text: ActiveWorkoutService.currentExercise ? ActiveWorkoutService.currentExercise.name : "No exercise"
                color: Theme.buttonText
                font.pixelSize: Theme.fontMedium
                font.bold: true
                anchors.centerIn: parent
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: ActiveWorkoutService.currentExercise

            ColumnLayout {
                width: parent.width
                spacing: Theme.spacing / 2

                Repeater {
                    model: ActiveWorkoutService.currentExercise ? ActiveWorkoutService.currentExercise.sets : []

                    Rectangle {
                        Layout.fillWidth: true
                        height: 60
                        radius: Theme.borderRadius
                        border.width: 1
                        border.color: Theme.border

                        color: {
                            if (ActiveWorkoutService.currentSet === modelData) {
                                Theme.primary
                            } else if (modelData.completed) {
                                Theme.success
                            } else {
                                Theme.surface
                            }
                        }

                        Behavior on color { ColorAnimation { duration: 200 } }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: Theme.padding
                            spacing: Theme.spacing

                            Text {
                                text: "Set " + (index + 1)
                                font.pixelSize: Theme.fontSmall
                                font.bold: true
                                color: ActiveWorkoutService.currentSet === modelData || modelData.completed ? Theme.buttonText : Theme.textPrimary
                                Layout.preferredWidth: 60
                            }

                            Text {
                                text: modelData.repetitions + " reps"
                                font.pixelSize: Theme.fontSmall
                                color: ActiveWorkoutService.currentSet === modelData || modelData.completed ? Theme.buttonText : Theme.textSecondary
                                Layout.preferredWidth: 80
                            }

                            Text {
                                text: modelData.weight + " kg"
                                font.pixelSize: Theme.fontSmall
                                color: ActiveWorkoutService.currentSet === modelData || modelData.completed ? Theme.buttonText : Theme.textSecondary
                                Layout.preferredWidth: 80
                            }

                            Item { Layout.fillWidth: true }

                            Rectangle {
                                width: 20
                                height: 20
                                radius: 10
                                color: (ActiveWorkoutService.currentSet === modelData || modelData.completed) ? Theme.buttonText : "transparent"
                                border.color: (modelData.completed) ? Theme.success : (ActiveWorkoutService.currentSet === modelData) ? Theme.primaryVariant : Theme.border
                                border.width: 2
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spacing

            Button {
                text: "◀ Prev"
                enabled: ActiveWorkoutService.isActive
                onClicked: ActiveWorkoutService.navigateToPrevious()
            }

            Button {
                text: "Done"
                enabled: ActiveWorkoutService.isActive && ActiveWorkoutService.currentSet
                background: Rectangle {
                    color: enabled ? Theme.success : Theme.border
                    radius: Theme.borderRadius
                }
                contentItem: Text {
                    text: qsTr("Done")
                    color: Theme.buttonText
                    font.pixelSize: Theme.fontMedium
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: ActiveWorkoutService.completeCurrentSet()
            }

            Button {
                text: "Next ▶"
                enabled: ActiveWorkoutService.isActive
                onClicked: ActiveWorkoutService.navigateToNext()
            }
        }
    }

    Connections {
        target: ActiveWorkoutService
        function onWorkoutCompleted() {
            stackView.pop()
        }
    }
}
