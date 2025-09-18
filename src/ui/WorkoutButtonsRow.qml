import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0

RowLayout {
    Layout.fillWidth: true
    spacing: Theme.spacing
    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

    signal importRoutines()

    Item {
        Layout.fillWidth: true
    }

    PrimaryButton {
        text: "Prompt"
        buttonTheme: Theme.buttonMedium
        buttonStyle: Theme.buttonStylePrimary
        onClicked: RoutineService.generateGptPrompt(WorkoutHistoryService.recentWorkoutsToJson())
    }

    PrimaryButton {
        text: "Import"
        buttonTheme: Theme.buttonMedium
        buttonStyle: Theme.buttonStylePrimary
        onClicked: importRoutines()
    }
}
