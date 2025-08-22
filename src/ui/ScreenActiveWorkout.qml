import QtQuick
import QtQuick.Controls
import LiftPlanner

Rectangle {
    id: root
    color: "#f0f0f0"

    Column {
        anchors.fill: parent
        spacing: 10
        padding: 15

        Text {
            text: ActiveWorkoutService.currentWorkout ? ActiveWorkoutService.currentWorkout.name : "Workout"
            font.pixelSize: 24
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            width: parent.width - 30
            height: 60
            color: "#3498db"
            radius: 8
            visible: ActiveWorkoutService.currentExercise

            Text {
                text: ActiveWorkoutService.currentExercise ?
                      ActiveWorkoutService.currentExercise.name : "No exercise"
                color: "white"
                font.pixelSize: 18
                anchors.centerIn: parent
            }
        }

        // Każda seria w osobnym rzędzie
        Column {
            width: parent.width - 30
            spacing: 5
            visible: ActiveWorkoutService.currentExercise

            Repeater {
                model: ActiveWorkoutService.currentExercise ? ActiveWorkoutService.currentExercise.sets : []

                Rectangle {
                    width: parent.width
                    height: 50
                    radius: 5
                    border.color: "#3498db"
                    border.width: 2

                    // Aktualna seria - podświetlona na niebiesko
                    color: ActiveWorkoutService.currentSet === modelData ? "#3498db" : "white"

                    Row {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 10

                        Text {
                            text: "Set " + (index + 1)
                            font.pixelSize: 14
                            font.bold: true
                            color: ActiveWorkoutService.currentSet === modelData ? "white" : "#333"
                            width: 50
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            text: modelData.repetitions + " reps"
                            font.pixelSize: 14
                            color: ActiveWorkoutService.currentSet === modelData ? "white" : "#666"
                            width: 70
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            text: modelData.weight + " kg"
                            font.pixelSize: 14
                            color: ActiveWorkoutService.currentSet === modelData ? "white" : "#666"
                            width: 60
                            verticalAlignment: Text.AlignVCenter
                        }

                        Rectangle {
                            width: 20
                            height: 20
                            radius: 10
                            color: ActiveWorkoutService.currentSet === modelData ? "white" : "transparent"
                            border.color: "#3498db"
                            border.width: 2
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                text: "◀ Prev"
                onClicked: ActiveWorkoutService.navigateToPrevious()
                enabled: ActiveWorkoutService.isActive
            }

            Button {
                text: "Done"
                onClicked: ActiveWorkoutService.completeCurrentSet()
                enabled: ActiveWorkoutService.isActive && ActiveWorkoutService.currentSet
                background: Rectangle {
                    color: "#27ae60"
                    radius: 5
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: "Next ▶"
                onClicked: ActiveWorkoutService.navigateToNext()
                enabled: ActiveWorkoutService.isActive
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
