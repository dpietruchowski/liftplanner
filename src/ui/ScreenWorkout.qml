import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

Rectangle {
    id: root
    color: Theme.background

    Component.onCompleted: WorkoutService.loadAllWorkouts()

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: Theme.padding

        ColumnLayout {
            id: columnLayout
            width: scrollView.contentItem.width
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacing

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacing
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

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
                    Layout.fillWidth: true
                    spacing: Theme.spacing
                    property bool expanded: false

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        radius: Theme.borderRadius
                        color: Theme.surface
                        border.color: Theme.primary
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            spacing: Theme.spacing

                            Rectangle {
                                Layout.preferredWidth: 28
                                Layout.preferredHeight: 28
                                radius: 14
                                color: Theme.primary
                                border.color: Theme.buttonText
                                border.width: 2

                                Text {
                                    anchors.centerIn: parent
                                    text: expanded ? "▼" : "▶"
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
                                Layout.preferredWidth: 28
                                Layout.preferredHeight: 28
                                radius: 14
                                color: Theme.primary

                                Text {
                                    anchors.centerIn: parent
                                    text: "▶"
                                    font.pixelSize: 14
                                    color: Theme.buttonText
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        ActiveWorkoutService.startWorkout(modelData)
                                        if (stackView.currentItem !== activeWorkoutScreen) {
                                            stackView.replace(activeWorkoutScreen)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Repeater {
                        model: expanded ? modelData.exercises : []

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            radius: Theme.borderRadius / 2
                            color: Theme.background

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                spacing: Theme.spacing
                                Layout.margins: Theme.padding

                                Text {
                                    text: modelData.name
                                    font.pixelSize: Theme.fontSmall
                                    font.bold: true
                                    color: Theme.textPrimary
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    text: "Sets: " + modelData.sets.map(function(set) { return set.repetitions }).join(", ")
                                    font.pixelSize: Theme.fontSmall
                                    color: Theme.textSecondary
                                    Layout.fillWidth: true
                                    horizontalAlignment: Text.AlignRight
                                    elide: Text.ElideRight
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Theme.border
                    }
                }
            }
        }
    }
}
