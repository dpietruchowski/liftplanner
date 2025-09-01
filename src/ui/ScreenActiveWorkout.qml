import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
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
            id: sv
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Column {
                id: contentColumn
                width: sv.availableWidth
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

            PrimaryButton {
                text: "Prev"
                enabled: ActiveWorkoutService.isActive
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStylePrimary
                onClicked: ActiveWorkoutService.navigateToPrevious()
            }

            PrimaryButton {
                text: "Done"
                enabled: ActiveWorkoutService.isActive && ActiveWorkoutService.currentSet
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStyleSuccess
                onClicked: {
                    ActiveWorkoutService.completeCurrentSet()
                    var rs = ActiveWorkoutService.currentExercise ? ActiveWorkoutService.currentExercise.restSeconds : 0
                    if (rs > 0) {
                        restDialog.show(rs, qsTr("Rest between sets"), false)
                    }
                }
            }

            PrimaryButton {
                text: "Next"
                enabled: ActiveWorkoutService.isActive
                buttonTheme: Theme.buttonMedium
                buttonStyle: Theme.buttonStylePrimary
                onClicked: ActiveWorkoutService.navigateToNext()
            }
        }

    }

    RestDialog {
        id: restDialog
        dialogVisible: false
        onDismissed: {
            restDialog.reset()
        }
    }

    Connections {
        target: ActiveWorkoutService
        function onWorkoutCompleted() {
            stackView.pop()
        }
    }
}
