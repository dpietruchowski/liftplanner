import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LiftPlanner

ColumnLayout {
    id: root
    spacing: Theme.spacing

    Text {
        text: qsTr("Workout History")
        font.pixelSize: Theme.fontLarge
        font.bold: true
        color: Theme.textPrimary
        Layout.alignment: Qt.AlignHCenter
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        ColumnLayout {
            id: workoutsColumn
            width: parent.width
            spacing: Theme.spacing / 2

            Repeater {
                model: WorkoutHistoryService.workouts

                delegate: ColumnLayout {
                    property var workout: modelData
                    width: parent.width
                    spacing: Theme.spacing / 2

                    Rectangle {
                        width: parent.width
                        height: 50
                        radius: Theme.borderRadius
                        color: Theme.surface
                        border.color: Theme.border
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Text {
                                text: workout.name
                                font.pixelSize: Theme.fontMedium
                                font.bold: true
                                color: Theme.textPrimary
                                Layout.alignment: Qt.AlignVCenter
                                elide: Text.ElideRight
                            }

                            Item { Layout.fillWidth: true }

                            Text {
                                text: workout.startedTime.toString("dd MMM yyyy hh:mm")
                                font.pixelSize: Theme.fontSmall
                                color: Theme.textSecondary
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: workoutDetails.visible = !workoutDetails.visible
                        }
                    }

                    ColumnLayout {
                        id: workoutDetails
                        width: parent.width
                        spacing: Theme.spacing / 2
                        visible: false

                        Repeater {
                            model: workout.exercises

                            delegate: ColumnLayout {
                                property var exercise: modelData
                                width: parent.width
                                spacing: Theme.spacing / 4

                                Rectangle {
                                    width: parent.width
                                    height: 40
                                    radius: Theme.borderRadius
                                    color: Theme.primary
                                    border.color: exercise.completed ? Theme.success : Theme.border
                                    border.width: 2

                                    Text {
                                        text: exercise.name
                                        anchors.centerIn: parent
                                        font.pixelSize: Theme.fontSmall
                                        color: Theme.buttonText
                                    }
                                }

                                Repeater {
                                    model: exercise.sets

                                    delegate: Rectangle {
                                        width: parent.width
                                        height: 32
                                        radius: Theme.borderRadius
                                        color: setData.completed ? Theme.success : Theme.surface
                                        border.color: Theme.border
                                        border.width: 1

                                        Text {
                                            text: "Set " + (index + 1) + ": " + setData.repetitions + " reps @ " + setData.weight + " kg"
                                            anchors.centerIn: parent
                                            font.pixelSize: Theme.fontSmall
                                            color: setData.completed ? Theme.buttonText : Theme.textPrimary
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
