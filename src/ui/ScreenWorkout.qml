import QtQuick
import QtQuick.Controls

import LiftPlanner

Rectangle {
    id: root
    color: "#f0f0f0"

    Component.onCompleted: {
        WorkoutService.loadAllWorkouts()
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: parent.width

        Column {
            width: parent.width
            spacing: 15
            padding: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Row {
                spacing: 10
                Button {
                    text: "Generate GPT Prompt"
                    onClicked: WorkoutService.generateGptPrompt()
                    background: Rectangle {
                        color: "#3498db"
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
                    text: "Import from JSON"
                    onClicked: WorkoutService.importWorkoutsFromClipboard()
                    background: Rectangle {
                        color: "#3498db"
                        radius: 5
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Repeater {
                model: WorkoutService.workouts

                Column {
                    width: parent.width - parent.padding * 2
                    spacing: 8

                    property bool expanded: true

                    Rectangle {
                        width: parent.width
                        height: 35
                        color: "#3498db"
                        radius: 5

                        Row {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 10

                            Rectangle {
                                width: 25
                                height: 25
                                radius: 12
                                color: "transparent"
                                border.color: "white"
                                border.width: 2
                                anchors.verticalCenter: parent.verticalCenter

                                Text {
                                    text: expanded ? "▼" : "▶"
                                    font.pixelSize: 12
                                    color: "white"
                                    anchors.centerIn: parent
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        expanded = !expanded
                                        console.log("Workout expanded:", expanded)
                                    }
                                }
                            }

                            Text {
                                text: modelData.name
                                font.pixelSize: 16
                                font.bold: true
                                color: "white"
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - 80
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                width: 25
                                height: 25
                                radius: 12
                                color: "white"
                                anchors.verticalCenter: parent.verticalCenter

                                Text {
                                    text: "▶"
                                    font.pixelSize: 12
                                    color: "#3498db"
                                    anchors.centerIn: parent
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        console.log("Starting workout:", modelData.name)
                                    }
                                }
                            }
                        }
                    }

                    Repeater {
                        model: expanded ? modelData.exercises : []
                        Rectangle {
                            width: parent.width
                            height: 30
                            color: "#ffffff"
                            radius: 3

                            function getRepetitionsList() {
                                return modelData.sets.map(function(set) {
                                    return set.repetitions;
                                });
                            }

                            Row {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 15

                                Text {
                                    text: modelData.name
                                    font.pixelSize: 14
                                    font.bold: true
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width * 0.45
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: "Sets: " + getRepetitionsList().join(", ")
                                    font.pixelSize: 12
                                    color: "#666666"
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width * 0.45
                                    horizontalAlignment: Text.AlignRight
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: "#dddddd"
                        visible: true
                    }
                }
            }
        }
    }
}
