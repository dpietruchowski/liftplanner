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

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                width: parent.width
                spacing: Theme.spacing / 2

                Repeater {
                    model: ActiveWorkoutService.currentWorkout ? ActiveWorkoutService.currentWorkout.exercises : []
                    delegate: ActiveWorkoutExerciseItem {
                        exercise: modelData
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