import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

RowLayout {
    Layout.fillWidth: true
    spacing: Theme.spacing.medium
    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

    signal importWorkouts()

    Item {
        Layout.fillWidth: true
    }

    ThemedButton {
        text: "Prompt"
        buttonSize: Theme.button.medium
        buttonStyle: Theme.button.primary
        onClicked: PlannedWorkoutService.generatePrompt()
    }

    ThemedButton {
        text: "Import"
        buttonSize: Theme.button.medium
        buttonStyle: Theme.button.primary
        onClicked: importWorkouts()
    }
}
