import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

Rectangle {
    id: root
    color: Theme.background

    Component.onCompleted: WorkoutService.loadAllWorkouts()

    ScrollView {
        anchors.fill: parent

        ColumnLayout {
            width: parent.width
            spacing: Theme.spacing
            Layout.alignment: Qt.AlignHCenter
            anchors.margins: Theme.padding

            RowLayout {
                spacing: Theme.spacing

                Button {
                    text: "Generate GPT Prompt"
                    onClicked: WorkoutService.generateGptPrompt()
                    background: Rectangle { color: Theme.primary; radius: Theme.borderRadius }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.buttonText
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Button {
                    text: "Import from JSON"
                    onClicked: WorkoutService.importWorkoutsFromClipboard()
                    background: Rectangle { color: Theme.primary; radius: Theme.borderRadius }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.buttonText
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Repeater {
                model: WorkoutService.workouts

                ColumnLayout {
                    width: parent.width
                    spacing: Theme.spacing
                    property bool expanded: false

                    Rectangle {
                        Layout.fillWidth: true
                        radius: Theme.borderRadius
                        color: Theme.surface
                        border.color: Theme.primary
                        border.width: 1
                        height: 50

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: Theme.padding
                            spacing: Theme.spacing

                            Rectangle {
                                width: 28
                                height: 28
                                radius: 14
                                color: Theme.primary
                                border.color: Theme.buttonText
                                border.width: 2

                                Text {
                                    text: expanded ? "▼" : "▶"
                                    anchors.centerIn: parent
                                    font.pixelSize: 14
                                    color: Theme.buttonText
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: expanded = !expanded
                                }
                            }

                            Text {
                                text: modelData.name
                                font.pixelSize: Theme.fontMedium
                                font.bold: true
                                color: Theme.textPrimary
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }

                            Rectangle {
                                width: 28
                                height: 28
                                radius: 14
                                color: Theme.primary

                                Text {
                                    text: "▶"
                                    anchors.centerIn: parent
                                    font.pixelSize: 14
                                    color: Theme.buttonText
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        ActiveWorkoutService.startWorkout(modelData)
                                        stackView.replace(activeWorkoutScreen)
                                    }
                                }
                            }
                        }
                    }

                    Repeater {
                        model: expanded ? modelData.exercises : []

                        Rectangle {
                            Layout.fillWidth: true
                            radius: Theme.borderRadius / 2
                            color: Theme.background
                            height: 40

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: Theme.padding
                                spacing: Theme.spacing

                                Text {
                                    text: modelData.name
                                    font.pixelSize: Theme.fontSmall
                                    font.bold: true
                                    color: Theme.textPrimary
                                    Layout.preferredWidth: parent.width * 0.45
                                    elide: Text.ElideRight
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    text: "Sets: " + modelData.sets.map(function(set) { return set.repetitions }).join(", ")
                                    font.pixelSize: Theme.fontSmall
                                    color: Theme.textSecondary
                                    Layout.preferredWidth: parent.width * 0.45
                                    horizontalAlignment: Text.AlignRight
                                    elide: Text.ElideRight
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
