import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner

ColumnLayout {
    Layout.fillWidth: true
    spacing: Theme.spacing
    property var workout
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
            anchors.margins: Theme.padding

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
                text: workout.name
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
                        ActiveWorkoutService.startWorkout(workout)
                        if (stackView.currentItem !== activeWorkoutScreen) {
                            stackView.replace(activeWorkoutScreen)
                        }
                    }
                }
            }
        }
    }

    Repeater {
        model: expanded ? workout.exercises : []
        delegate: ExerciseItem {
            Layout.fillWidth: true
            exercise: modelData
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: 1
        color: Theme.border
    }
}
